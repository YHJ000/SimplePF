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

	//리슨서버용 디버깅 중복 방지
	if (IsMappingAdded)
		return;

	if (auto* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (InputDataAsset->IMC.Get())
		{
			//캐릭터용 IMC 추가
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
			//캐릭터용 IMC 삭제
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
	InputComponent->BindAction(InputDataAsset->IA_Move.Get(), ETriggerEvent::Completed, this, &UMyCharInputComp::OnMoveRelease);	//정상 뗌
	InputComponent->BindAction(InputDataAsset->IA_Move.Get(), ETriggerEvent::Canceled, this, &UMyCharInputComp::OnMoveRelease);		//비정상 뗌

	InputComponent->BindAction(InputDataAsset->IA_Click.Get(), ETriggerEvent::Started, this, &UMyCharInputComp::OnClick);			//누름
	InputComponent->BindAction(InputDataAsset->IA_Click.Get(), ETriggerEvent::Completed, this, &UMyCharInputComp::OnClickRelease);	//뗌

	InputComponent->BindAction(InputDataAsset->IA_WPHandGun.Get(), ETriggerEvent::Started, this, &UMyCharInputComp::OnSwapHandGun);
	InputComponent->BindAction(InputDataAsset->IA_WPLifle.Get(), ETriggerEvent::Started, this, &UMyCharInputComp::OnSwapLifle);

	InputComponent->BindAction(InputDataAsset->IA_LifleScope.Get(), ETriggerEvent::Started, this, &UMyCharInputComp::OnLifleScope);				//누름
	InputComponent->BindAction(InputDataAsset->IA_LifleScope.Get(), ETriggerEvent::Completed, this, &UMyCharInputComp::OnLifleScopeRelease);	//뗌
}

void UMyCharInputComp::OnLook(const FInputActionValue& V)
{
	const FVector2D Axis = V.Get<FVector2D>();

	if (AUserChar* MyChar = Cast<AUserChar>(GetOwner()))
	{
		//언리얼이 서버로 자동으로 보내고, 서버에서 MoveComp가 회전 계산함
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

		//언리얼이 서버로 자동으로 보내고, 서버에서 MoveComp가 이동 계산함
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
			//캐릭터 방향 회전 따라가라고 다시 풀어줌
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
	//바뀐 무기는 서버가 알고 있고, 연출일 뿐이라 서버까지 필요없음
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
	//바뀐 무기는 서버가 알고 있고, 연출일 뿐이라 서버까지 필요없음
	if (AUserChar* MyChar = Cast<AUserChar>(GetOwner()))
	{
		MyChar->OnLifleScopeRelease();
	}
}