// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseChar.h"

#include "AIChar.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SIMPLEPF_API AAIChar : public ABaseChar
{
	GENERATED_BODY()

public:
	AAIChar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
