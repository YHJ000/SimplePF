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
	//�� ��Ʈ�ѷ� ȸ�� ����
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 60.f);

	//�ñ�⸸
	CameraComp->SetupAttachment(SpringArm);
	CameraComp->bUsePawnControlRotation = false;

	//�� ��Ʈ�ѷ� ȸ�� ����
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		//ĳ���� �̵� �������� ȸ��
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

	//������ �Է� �ʿ� ����
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

	//���� Ŭ�� �ʱ�ȭ��
	//Ŭ�� Possess�ǰų� ����۵� �� ȣ���(OnRep_Controller �ȿ� ��찡 �־ ���⼭ ó��)
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
				//ĳ���Ϳ� IMC �߰�
				InputComp->AddMappingContext(LocalPlayer);
			}
		}
	}
}

void AUserChar::PossessedBy(AController* NewController)
{
	//���� ���� �ݹ�
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
		//ĳ���Ϳ� IMC ����
		InputComp->RemoveMappingContext();
	}
}
