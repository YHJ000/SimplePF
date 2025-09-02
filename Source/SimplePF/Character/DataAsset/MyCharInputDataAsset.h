// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "MyCharInputDataAsset.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS(BlueprintType)
class SIMPLEPF_API UMyCharInputDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "UMyCharInputDataAsset")
	TObjectPtr<UInputMappingContext> IMC;

	UPROPERTY(EditDefaultsOnly, Category = "UMyCharInputDataAsset")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "UMyCharInputDataAsset")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "UMyCharInputDataAsset")
	TObjectPtr<UInputAction> IA_Click;

	UPROPERTY(EditDefaultsOnly, Category = "UMyCharInputDataAsset")
	TObjectPtr<UInputAction> IA_WPHandGun;

	UPROPERTY(EditDefaultsOnly, Category = "UMyCharInputDataAsset")
	TObjectPtr<UInputAction> IA_WPLifle;

	UPROPERTY(EditDefaultsOnly, Category = "UMyCharInputDataAsset")
	TObjectPtr<UInputAction> IA_LifleScope;
};
