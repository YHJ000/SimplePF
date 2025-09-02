// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseChar.h"

#include "UserChar.generated.h"

class UMyCharInputComp;
class UCameraComponent;
class USpringArmComponent;

UCLASS(Blueprintable)
class SIMPLEPF_API AUserChar : public ABaseChar
{
	GENERATED_BODY()

public:
	AUserChar();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

public:
	void OnLifleScope();
	void OnLifleScopeRelease();
	void SetCameraFOV(float FOV);

private:
	void ClearInputMappingContext();

public:
	FORCEINLINE const UMyCharInputComp* Get_InputComp() const { return InputComp.Get() ? InputComp.Get() : nullptr; }
	FORCEINLINE const UCameraComponent* Get_CameraComp() const { return CameraComp.Get() ? CameraComp.Get() : nullptr; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UserChar")
	TObjectPtr<UMyCharInputComp> InputComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UserChar")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UserChar")
	TObjectPtr<UCameraComponent> CameraComp;
};
