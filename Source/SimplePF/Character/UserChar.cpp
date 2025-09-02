// Fill out your copyright notice in the Description page of Project Settings.


#include "UserChar.h"
#include "EnhancedInputComponent.h"
#include "Framework/Player/PFPlayerController.h"
#include "Character/Components/MyCharInputComp.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Manager/UIManagerSubsystem.h"
#include "Character/Components/CharWeaponComp.h"
#include "SimplePF.h"

AUserChar::AUserChar()
{
	Tags.Add(TEXT("UserChar"));

	PrimaryActorTick.bCanEverTick = true;

	InputComp = CreateDefaultSubobject<UMyCharInputComp>(TEXT("InputComp"));
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArmComp"));
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));

	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 120.f;
	//폰 컨트롤러 회전 따라감
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 60.f);

	//맡기기만
	CameraComp->SetupAttachment(SpringArm);
	CameraComp->bUsePawnControlRotation = false;

	//폰 컨트롤러 회전 따라감
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		//캐릭터 이동 방향으로 회전
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->RotationRate = FRotator(0.f, 200.f, 0.f);
		MoveComp->bConstrainToPlane = true;
		MoveComp->bSnapToPlaneAtStart = true;
	}
}

void AUserChar::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("BeginPlay %s  Net=%d  IsLocal=%d  HasPC=%d"),
		*GetName(), (int32)GetWorld()->GetNetMode(), (int32)IsLocallyControlled(), (int32)(GetController() != nullptr));
}

void AUserChar::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearInputMappingContext();

	Super::EndPlay(EndPlayReason);
}

void AUserChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUserChar::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//서버는 입력 필요 없음
	if (!IsLocallyControlled())
		return;

	if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InputComp.Get())
		{
			InputComp->SetupPlayerInputComponent(EnhancedInputComp);
		}
	}
}

void AUserChar::PawnClientRestart()
{
	Super::PawnClientRestart();

	//소유 클라 초기화시
	//클라가 Possess되거나 재시작될 때 호출됨(OnRep_Controller 안올 경우가 있어서 여기서 처리)
	if (APFPlayerController* PlayerController = Cast<APFPlayerController>(GetController()))
	{
		UE_LOG(LogHJ, Warning, TEXT("[CLIENT] OnRep_Controller %s  PC=%s  IsLocal=%d"),
			*GetName(), *GetNameSafe(GetController()), (int32)IsLocallyControlled());

		if (!PlayerController->IsLocalController())
			return;

		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (InputComp.Get())
			{
				//캐릭터용 IMC 추가
				InputComp->AddMappingContext(LocalPlayer);
			}
		}
	}
}

void AUserChar::PossessedBy(AController* NewController)
{
	//서버 전용 콜백
	Super::PossessedBy(NewController);

	UE_LOG(LogTemp, Warning, TEXT("[SERVER] PossessedBy %s  PC=%s"),
		*GetName(), *GetNameSafe(NewController));
}

void AUserChar::UnPossessed()
{
	ClearInputMappingContext();

	Super::UnPossessed();
}

void AUserChar::OnLifleScope()
{
	if (!WeaponComp.Get())
		return;

	if (WeaponComp->GetCurWeaponType() != EWeaponType::WeaponType_Lifle)
		return;

	SetCameraFOV(45.f);

	if (auto* World = GetWorld())
	{
		if (auto* GameInst = World->GetGameInstance())
		{
			if (auto* UIManager = GameInst->GetSubsystem<UUIManagerSubsystem>())
			{
				UIManager->ShowWidget(EWidgetName::WBP_LifleScope);
			}
		}
	}
}

void AUserChar::OnLifleScopeRelease()
{
	SetCameraFOV(90.f);

	if (auto* World = GetWorld())
	{
		if (auto* GameInst = World->GetGameInstance())
		{
			if (auto* UIManager = GameInst->GetSubsystem<UUIManagerSubsystem>())
			{
				UIManager->HideWidget(EWidgetName::WBP_LifleScope);
			}
		}
	}
}

void AUserChar::SetCameraFOV(float FOV)
{
	if (!CameraComp.Get())
		return;

	CameraComp->FieldOfView = FOV;
}

void AUserChar::ClearInputMappingContext()
{
	if (InputComp.Get())
	{
		//캐릭터용 IMC 삭제
		InputComp->RemoveMappingContext();
	}
}
