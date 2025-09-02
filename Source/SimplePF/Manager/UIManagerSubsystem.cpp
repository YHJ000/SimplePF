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

	//GetGameViewport 없어서 기다렸다 로딩 붙이기
	Process_CreateLoadingPopup();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UUIManagerSubsystem::OnPreLoad);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UUIManagerSubsystem::OnPostLoad);

	//에디터/PIE 최초 진입(트래블 없이 생성된 월드) 보정
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
					//OnPostLoad 호출되기 전에 먼저 왔으면 거기서 처리해줌
				}
				else
				{
					//OnPostLoad 호출된 후에 온 경우(마지막임)
					//여기서 Loading지우고, Start 위젯 내부에서 생성
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
	//맵 로드 시작
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
	//맵 로드 끝남
	UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : OnPostLoad"));

	IsEnable_FirstMapLoaded = true;

	//TableManager Get_DelCompleteOrderTB 호출 전에 OnPostLoad가 먼저 불린 경우
	if (!IsEnable_WidgetTable)
		return;

	UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : OnPostLoad : Process_RemoveLoadingPopup"));

	//여기서 Loading지우고, Start 위젯 내부에서 생성
	Process_RemoveLoadingPopup(LoadedWorld);
}

void UUIManagerSubsystem::OnPostWorldInit(UWorld* LoadedWorld, const UWorld::InitializationValues IVS)
{
	if (!LoadedWorld)
		return;

	// PIE/Game 월드만 처리
	if (LoadedWorld->WorldType == EWorldType::PIE || LoadedWorld->WorldType == EWorldType::Game)
	{
		UE_LOG(LogHJ, Log, TEXT("OnPostWorldInit: %s"), *LoadedWorld->GetMapName());

		// 첫 진입 시(트래블 없이 생성)에도 로딩 종료 처리
		IsEnable_FirstMapLoaded = true;

		//TableManager Get_DelCompleteOrderTB 호출 전에 OnPostLoad가 먼저 불린 경우
		if (!IsEnable_WidgetTable)
			return;

		//여기서 Loading지우고, Start 위젯 내부에서 생성
		Process_RemoveLoadingPopup(LoadedWorld);
	}
}

TWeakObjectPtr<UBaseUserWidget> UUIManagerSubsystem::GetWidgetWeakPtr(EWidgetName WidgetName) const
{
	//일반 풀 체크
	if (const auto* ValuePtr = Using_Widget.Find(WidgetName))
	{
		if (IsValid(*ValuePtr))
		{
			return TWeakObjectPtr<UBaseUserWidget>(ValuePtr->Get());
		}
	}

	//재사용 풀 체크
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
		//로딩 팝업은 맵 전환이라서 따로 처리
		if (LoadingWidget.IsValid())
		{
			//존재하면 핸들까지 삭제
			Direct_RemoveLoadingPopup();
		}

		//새로 생성 시작
		Process_CreateLoadingPopup();
		return;
	}

	auto* ValuePtr = Recycle_Widget.Find(WidgetName);
	if (nullptr != ValuePtr && IsValid(*ValuePtr))
	{
		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : ShowWidget : Recycle_Widget IsValid"));

		//재사용 풀에 존재하면 키고, 일반 풀로 이동
		(*ValuePtr)->SetVisibility(ESlateVisibility::Visible);
		Using_Widget.Add(WidgetName, *ValuePtr);

		Recycle_Widget.Remove(WidgetName);
		return;
	}
	else
	{
		//재사용 풀에 없음

		//일반 풀에 있다면, Visible 처리 항상 되어 있음
		auto* ValuePtr1 = Using_Widget.Find(WidgetName);
		if (nullptr != ValuePtr1 && IsValid(*ValuePtr1))
			return;

		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : ShowWidget : Using_Widget IsValid"));

		//일반 풀에 생성
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

		//SoftPtr.IsValid는 생략, 첫 호출일 수 있음
		FSoftObjectPath Path = RowWidget->ClassUI.ToSoftObjectPath();
		if (!Path.IsValid())
		{
			UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : ShowWidget : TableName_Widget : !Path.IsValid()"));
			return;
		}

		//이미 로드중인 위젯이라면 패스
		if (ActiveHandles.Find(WidgetName))
			return;

		TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Path, FStreamableDelegate());
		if (Handle.IsValid())
		{
			//핸들 추가
			ActiveHandles.Add(WidgetName, Handle);

			if (Handle->HasLoadCompleted())
			{
				//메모리에 이미 있음
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
	//Hide면 토글형으로 사용한다는 말이니까
	if (EWidgetName::WBP_Loading == WidgetName)
	{
		//로딩 팝업은 맵 전환이라서 따로 처리
		return;
	}

	FName ConvertWidgetName = EnumUtil::GetNameByValue<EWidgetName>((int64)(WidgetName));

	//재사용 풀에 있는지 확인 후 존재하면 Hide
	auto* ValuePtr = Recycle_Widget.Find(WidgetName);
	if (nullptr != ValuePtr && IsValid(*ValuePtr))
	{
		(*ValuePtr)->SetVisibility(ESlateVisibility::Collapsed);

		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : HideWidget : Recycle_Widget : %s"), *ConvertWidgetName.ToString());
		return;
	}

	//재사용 풀에 없으면 일반 풀 체크해서 존재하면, 재사용 풀로 이동 후 Hide
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
	//완전 사용 안함

	if (EWidgetName::WBP_Loading == WidgetName)
	{
		//로딩 팝업은 맵 전환이라서 따로 처리
		return;
	}

	if (!GetGameInstance())
		return;

	UGameViewportClient* ViewPort = GetGameInstance()->GetGameViewportClient();
	if (nullptr == ViewPort)
		return;

	FName ConvertWidgetName = EnumUtil::GetNameByValue<EWidgetName>((int64)(WidgetName));

	//일반 풀 체크
	auto* ValuePtr = Using_Widget.Find(WidgetName);
	if (ValuePtr && IsValid(*ValuePtr))
	{
		RemoveViewportWidget(*ValuePtr);
		Using_Widget.Remove(WidgetName);

		UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : DestroyWidget : Using_Widget : %s"), *ConvertWidgetName.ToString());
		return;
	}

	//재사용 풀 체크
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
	//Initialize에서 GetGameViewport 없어서 기다렸다 로딩 붙이기
	TWeakObjectPtr<UUIManagerSubsystem> WeakUIManager = this;
	FTSTicker::GetCoreTicker().AddTicker(TEXT("UUIManagerSubsystem : Process_CreateLoadingPopup"), 0.0f,
		[WeakUIManager](float)
		-> bool {
			//자기자신이 없으면 중지
			if (!WeakUIManager.IsValid())
				return false;

			//이 아래로는 재시도
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

			//목적 달성했으니 중지
			return false;
		});
}

void UUIManagerSubsystem::Process_RemoveLoadingPopup(UWorld* LoadedWorld)
{
	//맵 로드 후에 맵마다 조건까지 기다렸다가 지우기
	TWeakObjectPtr<UUIManagerSubsystem> WeakUIManager = this;
	TWeakObjectPtr<UWorld> WeakLoadWorld = LoadedWorld;
	bool bDisable_ConditionCheck = false;
	int iEnableAddDelay = 200;	//iEnableAddDelay 프레임만큼 더 대기

	FTSTicker::GetCoreTicker().AddTicker(TEXT("UUIManagerSubsystem : Process_RemoveLoadingPopup"), 0.0f,
		[WeakUIManager, WeakLoadWorld, bDisable_ConditionCheck, iEnableAddDelay](float) mutable
		-> bool {
			//자기자신이 없으면 중지
			if (!WeakUIManager.IsValid() || !WeakLoadWorld.IsValid())
				return false;

			//조건 만족 전까지 재시도 진행-------------------------------------------------------------
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
					
					iEnableAddDelay = 0;	//스타트맵은 캐릭이 없음

					bDisable_ConditionCheck = true;
				}
				else if (LevelName.Equals(TEXT("SimplePFMap_Play")))
				{
					UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : Process_RemoveLoadingPopup : Condition Check : SimplePFMap_Play"));

					//카메라까지 잡힌 뒤에 로딩 팝업 삭제
					APlayerController* PC = UGameplayStatics::GetPlayerController(WeakLoadWorld.Get(), 0);
					if (!PC)
						return true;

					// Controller가 Pawn에 Possess 전
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

			//bDisable_ConditionCheck가 true로 여기까지 온거면 조건 만족 된거고, 프레임 대기 진행-------
			--iEnableAddDelay;

			//프레임 대기 끝났으면 로딩 팝업 삭제, 맵 별 위젯 생성 처리---------------------------------
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
				//프레임 대기 끝나기 전까지 대기
				UE_LOG(LogHJ, Log, TEXT("UUIManagerSubsystem : Process_RemoveLoadingPopup : iEnableAddDelay = %d"), iEnableAddDelay)

				return true;
			}

			//목적 달성했으니 중지
			return false;
		});
}

void UUIManagerSubsystem::Direct_CreateLoadingPopup()
{
	//AddViewport 상태면 리턴
	if (LoadingWidget.IsValid())
		return;

	if (!GetGameInstance())
		return;

	UGameViewportClient* ViewPort = GetGameInstance()->GetGameViewportClient();
	if (nullptr == ViewPort)
		return;

	//로딩 팝업
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

	//AddViewport 상태가 아니면 지울 필요 없음
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
			//비동기 타이밍상 중복 존재할수도 있으니까 한번 더 검사
			if (!Using_Widget.Contains(WidgetName))
			{
				if (UObject* LoadAsset = HandlePtr->Get()->GetLoadedAsset())
				{
					UClass* WidgetClass = Cast<UClass>(LoadAsset);
					if (WidgetClass && WidgetClass->IsChildOf(UBaseUserWidget::StaticClass()))
					{
						//생성
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

	//성공하던 못하던 핸들 처리
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
