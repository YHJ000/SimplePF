// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unique/GameMode/BaseGameMode.h"

#include "PlayGameMode.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SIMPLEPF_API APlayGameMode : public ABaseGameMode
{
	GENERATED_BODY()
	
protected:
	APlayGameMode();

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};
