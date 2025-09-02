// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/UIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameViewportClient.h"
#include "UI/BaseUserWidget.h"
#include "Engine/AssetManager.h"
#include "Manager/TableManagerSubsystem.h"
#include "Util/DefineEnum.h"
#include "Table/RowWidget.h"
#include "SimplePF.h"
#include "Containers/Ticker.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Character/UserChar.h"

UUIManagerSubsystem::UUIManagerSubsystem()
{
	ConstructorHelpers::FClassFinder<UBaseUserWidget> ClassWidget(TEXT("/Game/PF_UI/WBP_Loading.WBP_Loading_C"));
	if (ClassWidget.Succeeded())
	{
		LoadingUIClass = ClassWidget.Class;
	}
	else
	{
		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : WBP_Loading Class not Found"));
	}

	IsEnable_WidgetTable = false;
}

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : Initialize : Process_CreateLoadingPopup"));

	//GetGameViewport ��� ��ٷȴ� �ε� ���̱�
	Process_CreateLoadingPopup();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UUIManagerSubsystem::OnPreLoad);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UUIManagerSubsystem::OnPostLoad);

	//������/PIE ���� ����(Ʈ���� ���� ������ ����) ����
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UUIManagerSubsystem::OnPostWorldInit);

	if (!GetGameInstance())
		return;

	auto* TableManager = GetGameInstance()->GetSubsystem<UTableManagerSubsystem>();
	if (IsValid(TableManager))
	{
		TableManager->Get_DelCompleteOrderTB().AddWeakLambda(this, [this]()
			{
				UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : DelCompleteOrderTB"));
				IsEnable_WidgetTable = true;

				if (!IsEnable_FirstMapLoaded)
				{
					//OnPostLoad ȣ��Ǳ� ���� ���� ������ �ű⼭ ó������
				}
				else
				{
					//OnPostLoad ȣ��� �Ŀ� �� ���(��������)
					//���⼭ Loading�����, Start ���� ���ο��� ����
					if (GetWorld() && GetWorld()->GetName().Equals(TEXT("SimplePFMap_Start")))
					{
						Process_RemoveLoadingPopup(GetWorld());
					}
					else
					{
						Direct_RemoveLoadingPopup();
					}

					ShowWidget(EWidgetName::WBP_Start);
				}
			});
	}
}

void UUIManagerSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

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

void UUIManagerSubsystem::OnPreLoad(const FString& MapName)
{
	//�� �ε� ����
	UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : OnPreLoad"));

	for (const auto& Elem : Recycle_Widget)
	{
		if (!IsValid(Elem.Value))
			continue;

		if (!Elem.Value->ViewportHandle.IsValid())
			continue;

		RemoveViewportWidget(Elem.Value);
	}

	for (const auto& Elem : Using_Widget)
	{
		if (!IsValid(Elem.Value))
			continue;

		if (!Elem.Value->ViewportHandle.IsValid())
			continue;

		RemoveViewportWidget(Elem.Value);
	}

	Recycle_Widget.Empty();
	Using_Widget.Empty();
}

void UUIManagerSubsystem::OnPostLoad(UWorld* LoadedWorld)
{
	//�� �ε� ����
	UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : OnPostLoad"));

	IsEnable_FirstMapLoaded = true;

	//TableManager Get_DelCompleteOrderTB ȣ�� ���� OnPostLoad�� ���� �Ҹ� ���
	if (!IsEnable_WidgetTable)
		return;

	UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : OnPostLoad : Process_RemoveLoadingPopup"));

	//���⼭ Loading�����, Start ���� ���ο��� ����
	Process_RemoveLoadingPopup(LoadedWorld);
}

void UUIManagerSubsystem::OnPostWorldInit(UWorld* LoadedWorld, const UWorld::InitializationValues IVS)
{
	if (!LoadedWorld)
		return;

	// PIE/Game ���常 ó��
	if (LoadedWorld->WorldType == EWorldType::PIE || LoadedWorld->WorldType == EWorldType::Game)
	{
		UE_LOG(LogHJ, Log, TEXT("OnPostWorldInit: %s"), *LoadedWorld->GetMapName());

		// ù ���� ��(Ʈ���� ���� ����)���� �ε� ���� ó��
		IsEnable_FirstMapLoaded = true;

		//TableManager Get_DelCompleteOrderTB ȣ�� ���� OnPostLoad�� ���� �Ҹ� ���
		if (!IsEnable_WidgetTable)
			return;

		//���⼭ Loading�����, Start ���� ���ο��� ����
		Process_RemoveLoadingPopup(LoadedWorld);
	}
}

TWeakObjectPtr<UBaseUserWidget> UUIManagerSubsystem::GetWidgetWeakPtr(EWidgetName WidgetName) const
{
	//�Ϲ� Ǯ üũ
	if (const auto* ValuePtr = Using_Widget.Find(WidgetName))
	{
		if (IsValid(*ValuePtr))
		{
			return TWeakObjectPtr<UBaseUserWidget>(ValuePtr->Get());
		}
	}

	//���� Ǯ üũ
	if (const auto* ValuePtr1 = Recycle_Widget.Find(WidgetName))
	{
		if (IsValid(*ValuePtr1))
		{
			return TWeakObjectPtr<UBaseUserWidget>(ValuePtr1->Get());
		}
	}

	return TWeakObjectPtr<UBaseUserWidget>();
}

void UUIManagerSubsystem::ShowWidget(EWidgetName WidgetName, int32 ZOrder /*= 5*/)
{
	if (!IsEnable_WidgetTable)
		return;

	if (EWidgetName::WBP_Loading == WidgetName)
	{
		//�ε� �˾��� �� ��ȯ�̶� ���� ó��
		if (LoadingWidget.IsValid())
		{
			//�����ϸ� �ڵ���� ����
			Direct_RemoveLoadingPopup();
		}

		//���� ���� ����
		Process_CreateLoadingPopup();
		return;
	}

	auto* ValuePtr = Recycle_Widget.Find(WidgetName);
	if (nullptr != ValuePtr && IsValid(*ValuePtr))
	{
		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : ShowWidget : Recycle_Widget IsValid"));

		//���� Ǯ�� �����ϸ� Ű��, �Ϲ� Ǯ�� �̵�
		(*ValuePtr)->SetVisibility(ESlateVisibility::Visible);
		Using_Widget.Add(WidgetName, *ValuePtr);

		Recycle_Widget.Remove(WidgetName);
		return;
	}
	else
	{
		//���� Ǯ�� ����

		//�Ϲ� Ǯ�� �ִٸ�, Visible ó�� �׻� �Ǿ� ����
		auto* ValuePtr1 = Using_Widget.Find(WidgetName);
		if (nullptr != ValuePtr1 && IsValid(*ValuePtr1))
			return;

		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : ShowWidget : Using_Widget IsValid"));

		//�Ϲ� Ǯ�� ����
		if (!GetGameInstance())
			return;

		UTableManagerSubsystem* TableManager = GetGameInstance()->GetSubsystem<UTableManagerSubsystem>();
		if (nullptr == TableManager)
			return;

		FName ConvertWidgetName = EnumUtil::GetNameByValue<EWidgetName>((int64)WidgetName);
		const FRowWidget* RowWidget = TableManager->Find_TableRow<FRowWidget>(ETableName::TableName_Widget, ConvertWidgetName);
		if (nullptr == RowWidget)
		{
			UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : ShowWidget : TableName_Widget : nullptr == RowWidget"));
			return;
		}

		//SoftPtr.IsValid�� ����, ù ȣ���� �� ����
		FSoftObjectPath Path = RowWidget->ClassUI.ToSoftObjectPath();
		if (!Path.IsValid())
		{
			UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : ShowWidget : TableName_Widget : !Path.IsValid()"));
			return;
		}

		//�̹� �ε����� �����̶�� �н�
		if (ActiveHandles.Find(WidgetName))
			return;

		TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Path, FStreamableDelegate());
		if (Handle.IsValid())
		{
			//�ڵ� �߰�
			ActiveHandles.Add(WidgetName, Handle);

			if (Handle->HasLoadCompleted())
			{
				//�޸𸮿� �̹� ����
				LoadCompleteCreateWidget(WidgetName, ZOrder);
			}
			else
			{
				Handle->BindCompleteDelegate(FStreamableDelegate::CreateUObject(this, &UUIManagerSubsystem::LoadCompleteCreateWidget, WidgetName, ZOrder));
			}
		}
	}
}

void UUIManagerSubsystem::HideWidget(EWidgetName WidgetName)
{
	//Hide�� ��������� ����Ѵٴ� ���̴ϱ�
	if (EWidgetName::WBP_Loading == WidgetName)
	{
		//�ε� �˾��� �� ��ȯ�̶� ���� ó��
		return;
	}

	FName ConvertWidgetName = EnumUtil::GetNameByValue<EWidgetName>((int64)(WidgetName));

	//���� Ǯ�� �ִ��� Ȯ�� �� �����ϸ� Hide
	auto* ValuePtr = Recycle_Widget.Find(WidgetName);
	if (nullptr != ValuePtr && IsValid(*ValuePtr))
	{
		(*ValuePtr)->SetVisibility(ESlateVisibility::Collapsed);

		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : HideWidget : Recycle_Widget : %s"), *ConvertWidgetName.ToString());
		return;
	}

	//���� Ǯ�� ������ �Ϲ� Ǯ üũ�ؼ� �����ϸ�, ���� Ǯ�� �̵� �� Hide
	auto* ValuePtr1 = Using_Widget.Find(WidgetName);
	if (nullptr != ValuePtr1 && IsValid(*ValuePtr1))
	{
		Recycle_Widget.Add(WidgetName, *ValuePtr1);

		(*ValuePtr1)->SetVisibility(ESlateVisibility::Collapsed);
		Using_Widget.Remove(WidgetName);

		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : HideWidget : Using_Widget Move Recycle_Widget : %s"), *ConvertWidgetName.ToString());
		return;
	}
}

void UUIManagerSubsystem::DestroyWidget(EWidgetName WidgetName)
{
	//���� ��� ����

	if (EWidgetName::WBP_Loading == WidgetName)
	{
		//�ε� �˾��� �� ��ȯ�̶� ���� ó��
		return;
	}

	if (!GetGameInstance())
		return;

	UGameViewportClient* ViewPort = GetGameInstance()->GetGameViewportClient();
	if (nullptr == ViewPort)
		return;

	FName ConvertWidgetName = EnumUtil::GetNameByValue<EWidgetName>((int64)(WidgetName));

	//�Ϲ� Ǯ üũ
	auto* ValuePtr = Using_Widget.Find(WidgetName);
	if (ValuePtr && IsValid(*ValuePtr))
	{
		RemoveViewportWidget(*ValuePtr);
		Using_Widget.Remove(WidgetName);

		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : DestroyWidget : Using_Widget : %s"), *ConvertWidgetName.ToString());
		return;
	}

	//���� Ǯ üũ
	auto* ValuePtr1 = Recycle_Widget.Find(WidgetName);
	if (ValuePtr1 && IsValid(*ValuePtr1))
	{
		RemoveViewportWidget(*ValuePtr1);
		Recycle_Widget.Remove(WidgetName);
		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : DestroyWidget : Recycle_Widget : %s"), *ConvertWidgetName.ToString());
		return;
	}
}

void UUIManagerSubsystem::Process_CreateLoadingPopup()
{
	//Initialize���� GetGameViewport ��� ��ٷȴ� �ε� ���̱�
	TWeakObjectPtr<UUIManagerSubsystem> WeakUIManager = this;
	FTSTicker::GetCoreTicker().AddTicker(TEXT("UUIManagerSubsystem : Process_CreateLoadingPopup"), 0.0f,
		[WeakUIManager](float)
		-> bool {
			//�ڱ��ڽ��� ������ ����
			if (!WeakUIManager.IsValid())
				return false;

			//�� �Ʒ��δ� ��õ�
			auto* Instance = WeakUIManager->GetGameInstance();
			if (!Instance)
				return true;

			auto* UIManagerWorld = Instance->GetWorld();
			if (!UIManagerWorld)
				return true;

			if (!UIManagerWorld->GetGameViewport())
				return true;

			UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : Process_CreateLoadingPopup : Direct_CreateLoadingPopup"))
			WeakUIManager->Direct_CreateLoadingPopup();

			//���� �޼������� ����
			return false;
		});
}

void UUIManagerSubsystem::Process_RemoveLoadingPopup(UWorld* LoadedWorld)
{
	//�� �ε� �Ŀ� �ʸ��� ���Ǳ��� ��ٷȴٰ� �����
	TWeakObjectPtr<UUIManagerSubsystem> WeakUIManager = this;
	TWeakObjectPtr<UWorld> WeakLoadWorld = LoadedWorld;
	bool bDisable_ConditionCheck = false;
	int iEnableAddDelay = 200;	//iEnableAddDelay �����Ӹ�ŭ �� ���

	FTSTicker::GetCoreTicker().AddTicker(TEXT("UUIManagerSubsystem : Process_RemoveLoadingPopup"), 0.0f,
		[WeakUIManager, WeakLoadWorld, bDisable_ConditionCheck, iEnableAddDelay](float) mutable
		-> bool {
			//�ڱ��ڽ��� ������ ����
			if (!WeakUIManager.IsValid() || !WeakLoadWorld.IsValid())
				return false;

			//���� ���� ������ ��õ� ����-------------------------------------------------------------
			if (!WeakUIManager->IsEnable_WidgetTable)
				return true;

			if (!WeakUIManager->GetWorld() || !WeakUIManager->GetGameInstance())
				return true;

			if (!WeakUIManager->GetGameInstance()->GetGameViewportClient())
				return true;

			const FString LevelName = UGameplayStatics::GetCurrentLevelName(WeakLoadWorld.Get());

			if (!bDisable_ConditionCheck)
			{
				if (LevelName.Equals(TEXT("SimplePFMap_Start")))
				{
					UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : Process_RemoveLoadingPopup : Condition Check : SimplePFMap_Start"));
					
					iEnableAddDelay = 0;	//��ŸƮ���� ĳ���� ����

					bDisable_ConditionCheck = true;
				}
				else if (LevelName.Equals(TEXT("SimplePFMap_Play")))
				{
					UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : Process_RemoveLoadingPopup : Condition Check : SimplePFMap_Play"));

					//ī�޶���� ���� �ڿ� �ε� �˾� ����
					APlayerController* PC = UGameplayStatics::GetPlayerController(WeakLoadWorld.Get(), 0);
					if (!PC)
						return true;

					// Controller�� Pawn�� Possess ��
					AUserChar* MyChar = Cast<AUserChar>(PC->GetPawn());
					if (!MyChar)
						return true;

					if (PC->PlayerCameraManager && PC->PlayerCameraManager->PendingViewTarget.Target)
						return true;

					if (MyChar != PC->GetViewTarget())
						return true;

					const UCameraComponent* CameraComp = MyChar->Get_CameraComp();
					if (!CameraComp)
						return true;

					if (!CameraComp->IsActive())
						return true;

					bDisable_ConditionCheck = true;
				}
			}

			//bDisable_ConditionCheck�� true�� ������� �°Ÿ� ���� ���� �ȰŰ�, ������ ��� ����-------
			--iEnableAddDelay;

			//������ ��� �������� �ε� �˾� ����, �� �� ���� ���� ó��---------------------------------
			if (iEnableAddDelay <= 0)
			{
				if (LevelName.Equals(TEXT("SimplePFMap_Start")))
				{
					UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : Process_RemoveLoadingPopup : iEnableAddDelay <= 0 : SimplePFMap_Start"))

					WeakUIManager->Direct_RemoveLoadingPopup();
					WeakUIManager->ShowWidget(EWidgetName::WBP_Start);
				}
				else if (LevelName.Equals(TEXT("SimplePFMap_Play")))
				{
					UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : Process_RemoveLoadingPopup : iEnableAddDelay <= 0 : SimplePFMap_Play"))
					WeakUIManager->Direct_RemoveLoadingPopup();
					WeakUIManager->ShowWidget(EWidgetName::WBP_Play);
					WeakUIManager->ShowWidget(EWidgetName::WBP_HandGunScope);
				}
			}
			else
			{
				//������ ��� ������ ������ ���
				UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : Process_RemoveLoadingPopup : iEnableAddDelay = %d"), iEnableAddDelay)

				return true;
			}

			//���� �޼������� ����
			return false;
		});
}

void UUIManagerSubsystem::Direct_CreateLoadingPopup()
{
	//AddViewport ���¸� ����
	if (LoadingWidget.IsValid())
		return;

	if (!GetGameInstance())
		return;

	UGameViewportClient* ViewPort = GetGameInstance()->GetGameViewportClient();
	if (nullptr == ViewPort)
		return;

	//�ε� �˾�
	UBaseUserWidget* Create_LoadingWidget = CreateWidget<UBaseUserWidget>(GetWorld(), LoadingUIClass);
	if (Create_LoadingWidget)
	{
		TSharedRef<SWidget> SharedRefSWidget = Create_LoadingWidget->TakeWidget();
		ViewPort->AddViewportWidgetContent(SharedRefSWidget, 100);

		LoadingWidget = Create_LoadingWidget;
		LoadingWidget->WidgetName = EWidgetName::WBP_Loading;
		LoadingWidget->ViewportHandle = SharedRefSWidget;

		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : CreateLoadingPopup : AddViewportWidgetContent : WBP_Loading"));
	}
}

void UUIManagerSubsystem::Direct_RemoveLoadingPopup()
{
	if (!GetGameInstance())
		return;

	UGameViewportClient* ViewPort = GetGameInstance()->GetGameViewportClient();
	if (nullptr == ViewPort)
		return;

	//AddViewport ���°� �ƴϸ� ���� �ʿ� ����
	if (!LoadingWidget.IsValid())
		return;

	if (!LoadingWidget->ViewportHandle.IsValid())
		return;

	if (TSharedPtr<SWidget> SharedRef_Handle = LoadingWidget->ViewportHandle.Pin())
	{
		ViewPort->RemoveViewportWidgetContent(SharedRef_Handle.ToSharedRef());
		LoadingWidget->ViewportHandle.Reset();

		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : CreateLoadingPopup : RemoveViewportWidgetContent : WBP_Loading"));
	}

	LoadingWidget.Reset();
}

void UUIManagerSubsystem::LoadCompleteCreateWidget(EWidgetName WidgetName, int32 ZOrder)
{
	TSharedPtr<FStreamableHandle>* HandlePtr = ActiveHandles.Find(WidgetName);
	if (HandlePtr && HandlePtr->IsValid())
	{
		FName ConvertWidgetName = EnumUtil::GetNameByValue<EWidgetName>((int64)(WidgetName));
		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : LoadCompleteCreateWidget : %s"), *ConvertWidgetName.ToString());

		if (!HandlePtr->Get()->WasCanceled() && HandlePtr->Get()->HasLoadCompleted())
		{
			//�񵿱� Ÿ�ֻ̹� �ߺ� �����Ҽ��� �����ϱ� �ѹ� �� �˻�
			if (!Using_Widget.Contains(WidgetName))
			{
				if (UObject* LoadAsset = HandlePtr->Get()->GetLoadedAsset())
				{
					UClass* WidgetClass = Cast<UClass>(LoadAsset);
					if (WidgetClass && WidgetClass->IsChildOf(UBaseUserWidget::StaticClass()))
					{
						//����
						if (GetWorld() && GetGameInstance())
						{
							if (UGameViewportClient* ViewPort = GetGameInstance()->GetGameViewportClient())
							{
								if (UBaseUserWidget* Create_Widget = CreateWidget<UBaseUserWidget>(GetWorld(), WidgetClass, ConvertWidgetName))
								{
									TSharedRef<SWidget> SharedRefSWidget = Create_Widget->TakeWidget();
									ViewPort->AddViewportWidgetContent(SharedRefSWidget, ZOrder);

									UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : ShowWidget : AddViewportWidgetContent : %s"), *ConvertWidgetName.ToString());

									Create_Widget->WidgetName = WidgetName;
									Create_Widget->ViewportHandle = SharedRefSWidget;
									Using_Widget.Add(WidgetName, Create_Widget);
								}
							}
						}
					}
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

	ActiveHandles.Remove(WidgetName);
}

void UUIManagerSubsystem::RemoveViewportWidget(UBaseUserWidget* WidgetPtr)
{
	if (!GetGameInstance())
		return;

	UGameViewportClient* ViewPort = GetGameInstance()->GetGameViewportClient();
	if (!IsValid(ViewPort) || !IsValid(WidgetPtr) || !WidgetPtr->ViewportHandle.IsValid())
		return;

	if (TSharedPtr<SWidget> SharedRef_Handle = WidgetPtr->ViewportHandle.Pin())
	{
		ViewPort->RemoveViewportWidgetContent(SharedRef_Handle.ToSharedRef());
		WidgetPtr->ViewportHandle.Reset();

		FName ConvertWidgetName = EnumUtil::GetNameByValue<EWidgetName>((int64)(WidgetPtr->WidgetName));
		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : RemoveViewportWidget : RemoveViewportWidgetContent : %s"), *ConvertWidgetName.ToString());

		auto TableManager = GetGameInstance()->GetSubsystem<UTableManagerSubsystem>();
		if (TableManager)
		{
			const FRowWidget* TableRow = TableManager->Find_TableRow<FRowWidget>(ETableName::TableName_Widget, ConvertWidgetName);
			if (TableRow)
			{
				FSoftObjectPath Path = TableRow->ClassUI.ToSoftObjectPath();
				if (Path.IsValid()) 
				{
					UAssetManager::GetStreamableManager().Unload(Path);
					UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : RemoveViewportWidget : StreamableManager().Unload : %s"), *ConvertWidgetName.ToString());
				}
			}
		}
	}
}
