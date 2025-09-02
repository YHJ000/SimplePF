// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "PFPlayerController.generated.h"

class UPFPlayerInputDataAsset;

UCLASS(Blueprintable)
class SIMPLEPF_API APFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APFPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void OnEsc();

private:
	UPROPERTY(EditDefaultsOnly, Category = "APFPlayerController", meta = (AllowPrivateAccess = true))
	TObjectPtr<UPFPlayerInputDataAsset> InputDataAsset;
};
