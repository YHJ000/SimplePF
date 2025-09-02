// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DefineEnum.h"

#include "BaseCharAnimInstance.generated.h"

class ABaseChar;
class USceneComponent;

UCLASS(Blueprintable)
class SIMPLEPF_API UBaseCharAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	void Update_AnimState(EAnimState AnimState);
	FORCEINLINE void Update_Speed(float speed) { Speed = speed; }
	FORCEINLINE void Update_Right(float right) { Right = right; }
	FORCEINLINE void Update_IsDie(bool bDie) { IsDie = bDie; }

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimInstance")
	TObjectPtr<UAnimMontage> Montage_Hit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimInstance")
	TObjectPtr<UAnimMontage> Montage_Shoot;

	//FSM Ä³½Ì¿ë
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimInstance")
	EAnimState FSM_State;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimInstance")
	float Speed = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimInstance")
	float Right = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimInstance")
	bool IsDie = false;
};
