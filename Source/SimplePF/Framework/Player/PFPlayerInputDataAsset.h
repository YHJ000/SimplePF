// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "PFPlayerInputDataAsset.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS(BlueprintType)
class SIMPLEPF_API UPFPlayerInputDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "UPFPlayerInputDataAsset")
	TObjectPtr<UInputMappingContext> IMC;

	UPROPERTY(EditDefaultsOnly, Category = "UPFPlayerInputDataAsset")
	TObjectPtr<UInputAction> IA_Esc;
};
