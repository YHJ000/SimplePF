// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SimplePFGameInstance.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SIMPLEPF_API USimplePFGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
protected:
	virtual void Init() override;

};
