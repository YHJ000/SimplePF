// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Util/DefineEnum.h"
#include "Engine/StreamableManager.h"

#include "TableManagerSubsystem.generated.h"

class UDataTable;

DECLARE_MULTICAST_DELEGATE(FOnCompleteLoad)

UCLASS(Blueprintable)
class SIMPLEPF_API UTableManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UTableManagerSubsystem();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	//�ε� ���� ���� ���̺� �Ϸ� �ƴ���
	FORCEINLINE bool Get_IsComplete_OrderLoad() const { return IsComplete_OrderLoad; }

	//�Ϸ� �ݹ� ���ε��� ��������Ʈ
	FORCEINLINE FOnCompleteLoad& Get_DelCompleteOrderTB() { return Del_CompleteOrderTB; };

public:
	template<typename T>
	FORCEINLINE const T* Find_TableRow(ETableName TableType, FName SearchName) const
	{
		if (const UDataTable* Data = LoadDataTable.FindRef(TableType))
		{
			return Data->FindRow<T>(SearchName, FString::Printf(TEXT("Find_TableRow : Table = %d, Row = %s"), static_cast<int32>(TableType), *SearchName.ToString()));
		}

		return nullptr;
	}

private:
	void StartLoadTableList();
	void LoadCompleteLoadTableList();

	//���� ���� �ε�
	void Load_OrderTable();
	void Per_CompleteLoad_OrderTable(ETableName TableName);

	//õõ�� �ε�
	void Load_UnOrderTable();
	void Per_CompleteLoad_UnOrderTable(ETableName TableName);
	
private:
	//������ ���� �ε��ؾ��ϴ� ���̺� �ε� �Ϸ��ߴ���
	bool IsComplete_OrderLoad = false;
	FOnCompleteLoad Del_CompleteOrderTB;

	//��� ���̺� �ε� �Ϸ�
	bool IsComplete_AllLoad = false;

	TSharedPtr<FStreamableHandle> TB_LoadTable_Handle;

	//�ε��� ���̺� ��� ���̺�
	UPROPERTY(EditDefaultsOnly, Category = "TableManager", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UDataTable> TB_LoadTable;

	TQueue<TPair<ETableName, TSoftObjectPtr<UDataTable>>> Queue_OrderLoad;
	TQueue<TPair<ETableName, TSoftObjectPtr<UDataTable>>> Queue_UnOrderLoad;

	//������� �񵿱� �ε� �ڵ�
	TMap<ETableName, TSharedPtr<FStreamableHandle>> ActiveHandles;

	//<���̺� �̸� <Ž�� �̸�, ������>>
	UPROPERTY(Transient)
	TMap<ETableName,UDataTable*> LoadDataTable;
};
