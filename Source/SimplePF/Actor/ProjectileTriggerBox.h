// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"

#include "ProjectileTriggerBox.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLEPF_API AProjectileTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnEndOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
