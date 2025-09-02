// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/MyCharInputComp.h"
#include "Character/DataAsset/MyCharInputDataAsset.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Character/UserChar.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Components/CharWeaponComp.h"
#include "Character/Components/CharAnimStateComp.h"

UMyCharInputComp::UMyCharInputComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMyCharInputComp::BeginPlay()
{
	Super::BeginPlay();
}

void UMyCharInputComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMyCharInputComp::AddMappingContext(ULocalPlayer* LocalPlayer)
{
	if (nullptr == LocalPlayer || !InputDataAsset.Get())
		return;

	//���������� ����� �ߺ� ����
	if (IsMappingAdded)
		return;

	if (auto* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (InputDataAsset->IMC.Get())
		{
			//ĳ���Ϳ� IMC �߰�
			Subsystem->AddMappingContext(InputDataAsset->IMC.Get(), 0);
			LocalPlayerCaches = LocalPlayer;
			IsMappingAdded = true;
		}
	}
}

void UMyCharInputComp::RemoveMappingContext()
{
	if (nullptr == LocalPlayerCaches || !InputDataAsset.Get())
		return;

	if (auto* Subsystem = LocalPlayerCaches->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (InputDataAsset->IMC.Get())
		{
			//ĳ���Ϳ� IMC ����
			Subsystem->RemoveMappingContext(InputDataAsset->IMC.Get());
			LocalPlayerCaches = nullptr;
			IsMappingAdded = false;
		}
	}
}

void UMyCharInputComp::SetupPlayerInputComponent(UEnhancedInputComponent* InputComponent)
{
	if (nullptr == InputComponent || !InputDataAsset.Get() || !InputDataAsset->IA_Look.Get()
		|| !InputDataAsset->IA_Move.Get() || !InputDataAsset->IA_Click.Get() 
		|| !InputDataAsset->IA_WPHandGun.Get() || !InputDataAsset->IA_WPLifle.Get())
		return;

	InputComponent->BindAction(InputDataAsset->IA_Look.Get(), ETriggerEvent::Triggered, this, &UMyCharInputComp::OnLook);

	InputComponent->BindAction(InputDataAsset->IA_Move.Get(), ETriggerEvent::Triggered, this, &UMyCharInputComp::OnMove);
	InputComponent->BindAction(InputDataAsset->IA_Move.Get(), ETriggerEvent::Completed, this, &UMyCharInputComp::OnMoveRelease);	//���� ��
	InputComponent->BindAction(InputDataAsset->IA_Move.Get(), ETriggerEvent::Canceled, this, &UMyCharInputComp::OnMoveRelease);		//������ ��

	InputComponent->BindAction(InputDataAsset->IA_Click.Get(), ETriggerEvent::Started, this, &UMyCharInputComp::OnClick);			//����
	InputComponent->BindAction(InputDataAsset->IA_Click.Get(), ETriggerEvent::Completed, this, &UMyCharInputComp::OnClickRelease);	//��

	InputComponent->BindAction(InputDataAsset->IA_WPHandGun.Get(), ETriggerEvent::Started, this, &UMyCharInputComp::OnSwapHandGun);
	InputComponent->BindAction(InputDataAsset->IA_WPLifle.Get(), ETriggerEvent::Started, this, &UMyCharInputComp::OnSwapLifle);

	InputComponent->BindAction(InputDataAsset->IA_LifleScope.Get(), ETriggerEvent::Started, this, &UMyCharInputComp::OnLifleScope);				//����
	InputComponent->BindAction(InputDataAsset->IA_LifleScope.Get(), ETriggerEvent::Completed, this, &UMyCharInputComp::OnLifleScopeRelease);	//��
}

void UMyCharInputComp::OnLook(const FInputActionValue& V)
{
	const FVector2D Axis = V.Get<FVector2D>();

	if (AUserChar* MyChar = Cast<AUserChar>(GetOwner()))
	{
		//�𸮾��� ������ �ڵ����� ������, �������� MoveComp�� ȸ�� �����
		MyChar->AddControllerYawInput(Axis.X);
		MyChar->AddControllerPitchInput(Axis.Y);
	}
}

void UMyCharInputComp::OnMove(const FInputActionValue& V)
{
	const FVector2D Axis = V.Get<FVector2D>();

	if (AUserChar* MyChar = Cast<AUserChar>(GetOwner()))
	{
		const FRotator YawOnly(0.f, MyChar->GetControlRotation().Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);

		//�𸮾��� ������ �ڵ����� ������, �������� MoveComp�� �̵� �����
		MyChar->AddMovementInput(Forward, Axis.Y);
		MyChar->AddMovementInput(Right, Axis.X);	
	}
}

void UMyCharInputComp::OnMoveRelease(const FInputActionValue& V)
{
	//Input Zero
	if (AUserChar* MyChar = Cast<AUserChar>(GetOwner()))
	{
		UCharacterMovementComponent* MoveComp = MyChar->GetCharacterMovement();
		if (MoveComp)
		{
			//ĳ���� ���� ȸ�� ���󰡶�� �ٽ� Ǯ����
			MoveComp->bOrientRotationToMovement = true;
		}
	}
}

void UMyCharInputComp::OnClick()
{
	if (AUserChar* MyChar = Cast<AUserChar>(GetOwner()))
	{
		if (UCharAnimStateComp* AnimFSMComp = MyChar->Get_AnimFSMComp())
		{
			AnimFSMComp->Server_Shoot();
		}
	}
}

void UMyCharInputComp::OnClickRelease()
{

}

void UMyCharInputComp::OnSwapHandGun()
{
	if (AUserChar* MyChar = Cast<AUserChar>(GetOwner()))
	{
		if (UCharWeaponComp* WeaponComp = MyChar->Get_WeaponComp())
		{
			WeaponComp->Server_ChangeWeapon(EWeaponType::WeaponType_HandGun);
		}
	}
}

void UMyCharInputComp::OnSwapLifle()
{
	if (AUserChar* MyChar = Cast<AUserChar>(GetOwner()))
	{
		if (UCharWeaponComp* WeaponComp = MyChar->Get_WeaponComp())
		{
			WeaponComp->Server_ChangeWeapon(EWeaponType::WeaponType_Lifle);
		}
	}
}

void UMyCharInputComp::OnLifleScope()
{
	//�ٲ� ����� ������ �˰� �ְ�, ������ ���̶� �������� �ʿ����
	if (AUserChar* MyChar = Cast<AUserChar>(GetOwner()))
	{
		if (UCharWeaponComp* WeaponComp = MyChar->Get_WeaponComp())
		{
			if (EWeaponType::WeaponType_Lifle != WeaponComp->GetCurWeaponType())
				return;

			MyChar->OnLifleScope();
		}
	}
}

void UMyCharInputComp::OnLifleScopeRelease()
{
	//�ٲ� ����� ������ �˰� �ְ�, ������ ���̶� �������� �ʿ����
	if (AUserChar* MyChar = Cast<AUserChar>(GetOwner()))
	{
		MyChar->OnLifleScopeRelease();
	}
}