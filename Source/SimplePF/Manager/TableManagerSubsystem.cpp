// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/TableManagerSubsystem.h"
#include "Engine/AssetManager.h"
#include "Table/RowLoadTable.h"
#include "StructUtils/InstancedStruct.h"
#include "Engine/DataTable.h"
#include "SimplePF.h"
#include "Manager/UIManagerSubsystem.h"

UTableManagerSubsystem::UTableManagerSubsystem()
{
	TB_LoadTable = FSoftObjectPath(TEXT("/Game/PF_Table/Table_LoadTable.Table_LoadTable"));
}

void UTableManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Initialize"));

	//UIManagerSubsystem Init ���� �ϰ�, TableManagerSubsystem InitialzieŸ�� ��������Ʈ �ݹ� ���޵�
	Collection.InitializeDependency<UUIManagerSubsystem>();

	UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Initialize : UTableManagerSubsystem NextIn"));

	StartLoadTableList();
}

void UTableManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	for (auto& HandlePair : ActiveHandles)
	{
		if (HandlePair.Value.IsValid())
		{
			HandlePair.Value->CancelHandle();
			HandlePair.Value->ReleaseHandle();
		}
	}
	ActiveHandles.Empty();
}

void UTableManagerSubsystem::StartLoadTableList()
{
	if (TB_LoadTable.IsNull())
	{
		UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : StartLoadTableList : TB_LoadTable.IsNull()"));

		IsComplete_OrderLoad = true;
		Del_CompleteOrderTB.Broadcast();
		return;
	}

	//�ε��� ���̺� ��� ��ü�� ������ Order ���̺� �ε� �н�
	const FSoftObjectPath Path = TB_LoadTable.ToSoftObjectPath();
	if (!Path.IsValid())
	{
		UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : StartLoadTableList : !Path.IsValid()"));

		IsComplete_OrderLoad = true;
		Del_CompleteOrderTB.Broadcast();
		return;
	}

	UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : StartLoadTableList : RequestAsyncLoad"));
	TB_LoadTable_Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Path, FStreamableDelegate());
	if (TB_LoadTable_Handle.IsValid())
	{
		if (TB_LoadTable_Handle->HasLoadCompleted())
		{
			LoadCompleteLoadTableList();
		}
		else
		{
			TB_LoadTable_Handle->BindCompleteDelegate(FStreamableDelegate::CreateUObject(this, &UTableManagerSubsystem::LoadCompleteLoadTableList));
		}
	}

}

void UTableManagerSubsystem::LoadCompleteLoadTableList()
{
	if (TB_LoadTable_Handle.IsValid() && !TB_LoadTable_Handle->WasCanceled() && TB_LoadTable_Handle->HasLoadCompleted())
	{
		if (UObject* Asset = TB_LoadTable_Handle->GetLoadedAsset())
		{
			if (UDataTable* LoadTable = Cast<UDataTable>(Asset))
			{
				TArray<FRowLoadTable*> ArrRow_LoadTable;
				LoadTable->GetAllRows(TEXT("Load_OrderTable"), ArrRow_LoadTable);

				if (0 < ArrRow_LoadTable.Num())
				{
					for (const auto& Elem : ArrRow_LoadTable)
					{
						FName ConvertName = EnumUtil::GetNameByValue<ETableName>((int64)(Elem->TableName));
						UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : LoadCompleteLoadTableList : %s"), *ConvertName.ToString());

						if (Elem->NeedLoadOrder)
						{
							Queue_OrderLoad.Enqueue(TPair<ETableName, TSoftObjectPtr<UDataTable>>(Elem->TableName, Elem->ObjectTable));
						}
						else
						{
							Queue_UnOrderLoad.Enqueue(TPair<ETableName, TSoftObjectPtr<UDataTable>>(Elem->TableName, Elem->ObjectTable));
						}
					}

				}
			}
		}
		TB_LoadTable_Handle->ReleaseHandle();
		TB_LoadTable_Handle.Reset();
	}

	Load_OrderTable();
}

void UTableManagerSubsystem::Load_OrderTable()
{
	//���� ���Ѿ��ϴ� ���̺�

	if (Queue_OrderLoad.IsEmpty())
	{
		UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Load_OrderTable : IsEmpty"));

		//���� ���̺� �Ϸ�, ��������Ʈ
		if (!IsComplete_OrderLoad)
		{
			UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Load_OrderTable : Broadcast"));

			IsComplete_OrderLoad = true;
			Del_CompleteOrderTB.Broadcast();
		}

		//���� �����ѵ� �Ǵ� ���̺� �ε�
		Load_UnOrderTable();
		return;
	}

	//�ε� ó��
	TPair<ETableName, TSoftObjectPtr<UDataTable>> Pair_LoadTable;
	Queue_OrderLoad.Dequeue(Pair_LoadTable);

	const FSoftObjectPath Path = Pair_LoadTable.Value.ToSoftObjectPath();
	if (!Path.IsValid())
	{
		UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Load_OrderTable : !Path.IsValid()"));

		//��� ���� �̻��ϸ�, ���� ���̺� �ε�
		Load_OrderTable();
		return;
	}

	//�̹� �ε����� �����̶��, ���� ���̺� �ε�
	if (ActiveHandles.Find(Pair_LoadTable.Key))
	{
		Load_OrderTable();
		return;
	}

	FName ConvertName = EnumUtil::GetNameByValue<ETableName>((int64)(Pair_LoadTable.Key));
	UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Load_OrderTable : RequestAsyncLoad : %s"), *ConvertName.ToString());

	TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Path, FStreamableDelegate());
	if (Handle.IsValid())
	{
		//�ڵ� �߰�
		ActiveHandles.Add(Pair_LoadTable.Key, Handle);

		if (Handle->HasLoadCompleted())
		{
			//�޸𸮿� �̹� ����
			Per_CompleteLoad_OrderTable(Pair_LoadTable.Key);
		}
		else
		{
			Handle->BindCompleteDelegate(FStreamableDelegate::CreateUObject(this, &UTableManagerSubsystem::Per_CompleteLoad_OrderTable, Pair_LoadTable.Key));
		}
	}
}

void UTableManagerSubsystem::Per_CompleteLoad_OrderTable(ETableName TableName)
{
	TSharedPtr<FStreamableHandle>* HandlePtr = ActiveHandles.Find(TableName);
	if (HandlePtr && HandlePtr->IsValid())
	{
		if (!HandlePtr->Get()->WasCanceled() && HandlePtr->Get()->HasLoadCompleted())
		{
			if (UObject* Asset = HandlePtr->Get()->GetLoadedAsset())
			{
				if (UDataTable* LoadTable = Cast<UDataTable>(Asset))
				{
					FName ConvertName = EnumUtil::GetNameByValue<ETableName>((int64)(TableName));
					UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Per_CompleteLoad_OrderTable : %s"), *ConvertName.ToString());

					LoadDataTable.Add(TableName, LoadTable);
				}
			}
		}
	}

	//�����ϴ� ���ϴ� �ڵ� ó��
	if (HandlePtr && HandlePtr->IsValid())
	{
		HandlePtr->Get()->ReleaseHandle();
		HandlePtr->Reset();
	}

	ActiveHandles.Remove(TableName);

	//���� ���̺� �ε�
	Load_OrderTable();
}

void UTableManagerSubsystem::Load_UnOrderTable()
{
	//���� �����ѵ� �Ǵ� ���̺� ����

	if (Queue_UnOrderLoad.IsEmpty())
	{
		UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Load_UnOrderTable : IsEmpty"));

		//��� ���̺� �ε� �Ϸ�
		IsComplete_AllLoad = true;
		return;
	}

	//�ε� ó��
	TPair<ETableName, TSoftObjectPtr<UDataTable>> Pair_LoadTable;
	Queue_UnOrderLoad.Dequeue(Pair_LoadTable);

	const FSoftObjectPath Path = Pair_LoadTable.Value.ToSoftObjectPath();
	if (!Path.IsValid())
	{
		UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Load_UnOrderTable : !Path.IsValid()"));

		//���� ���̺� �ε�
		Load_UnOrderTable();
		return;
	}

	//�̹� �ε����� �����̶��, ���� ���̺� �ε�
	if (ActiveHandles.Find(Pair_LoadTable.Key))
	{
		Load_UnOrderTable();
		return;
	}

	FName ConvertName = EnumUtil::GetNameByValue<ETableName>((int64)(Pair_LoadTable.Key));
	UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Load_UnOrderTable : RequestAsyncLoad : %s"), *ConvertName.ToString());

	TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Path, FStreamableDelegate());
	if (Handle.IsValid())
	{
		//�ڵ� �߰�
		ActiveHandles.Add(Pair_LoadTable.Key, Handle);

		if (Handle->HasLoadCompleted())
		{
			//�޸𸮿� �̹� ����
			Per_CompleteLoad_UnOrderTable(Pair_LoadTable.Key);
		}
		else
		{
			Handle->BindCompleteDelegate(FStreamableDelegate::CreateUObject(this, &UTableManagerSubsystem::Per_CompleteLoad_UnOrderTable, Pair_LoadTable.Key));
		}
	}
}

void UTableManagerSubsystem::Per_CompleteLoad_UnOrderTable(ETableName TableName)
{
	TSharedPtr<FStreamableHandle>* HandlePtr = ActiveHandles.Find(TableName);
	if (HandlePtr && HandlePtr->IsValid())
	{
		if (!HandlePtr->Get()->WasCanceled() && HandlePtr->Get()->HasLoadCompleted())
		{
			if (UObject* Asset = HandlePtr->Get()->GetLoadedAsset())
			{
				if (UDataTable* LoadTable = Cast<UDataTable>(Asset))
				{
					FName ConvertName = EnumUtil::GetNameByValue<ETableName>((int64)(TableName));
					UE_LOG(LogHJ, Log, TEXT("UTableManagerSubsystem : Per_CompleteLoad_UnOrderTable : %s"), *ConvertName.ToString());

					LoadDataTable.Add(TableName, LoadTable);
				}
			}
		}
	}

	//�����ϴ� ���ϴ� �ڵ� ó��
	if (HandlePtr && HandlePtr->IsValid())
	{
		HandlePtr->Get()->ReleaseHandle();
		HandlePtr->Reset();
	}

	ActiveHandles.Remove(TableName);

	//���� ���̺� �ε�
	Load_UnOrderTable();
}
