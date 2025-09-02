// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Player/PFPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Framework/Player/PFPlayerInputDataAsset.h"

APFPlayerController::APFPlayerController()
{

}

void APFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController() || !InputDataAsset.Get())
		return;

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (auto* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (const UInputMappingContext* CIMC = InputDataAsset->IMC.Get())
			{
				//로컬 IMC 추가
				Subsystem->AddMappingContext(CIMC, 10);
			}
		}
	}
}

void APFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalController() || !InputDataAsset.Get())
		return;

	if (UEnhancedInputComponent* InputComp = Cast<UEnhancedInputComponent>(InputComponent))
	{
		InputComp->BindAction(InputDataAsset->IA_Esc.Get(), ETriggerEvent::Started, this, &APFPlayerController::OnEsc);
	}
}

void APFPlayerController::OnEsc()
{

}
