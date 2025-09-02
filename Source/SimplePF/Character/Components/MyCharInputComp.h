// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"

#include "MyCharInputComp.generated.h"

class ULocalPlayer;
class UEnhancedInputComponent;
class UMyCharInputDataAsset;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIMPLEPF_API UMyCharInputComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMyCharInputComp();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void AddMappingContext(ULocalPlayer* LocalPlayer);
	void RemoveMappingContext();
	void SetupPlayerInputComponent(UEnhancedInputComponent* InputComponent);

private:
	void OnLook(const FInputActionValue& V);
	void OnMove(const FInputActionValue& V);
	void OnMoveRelease(const FInputActionValue& V);
	void OnClick();
	void OnClickRelease();
	void OnSwapHandGun();
	void OnSwapLifle();
	void OnLifleScope();
	void OnLifleScopeRelease();

private:
	UPROPERTY(transient)
	bool IsMappingAdded = false;

	//Ä³½Ã¿ë
	UPROPERTY(transient)
	TWeakObjectPtr<ULocalPlayer> LocalPlayerCaches;

	UPROPERTY(EditDefaultsOnly, Category = "UMyCharInputComp")
	TObjectPtr<UMyCharInputDataAsset> InputDataAsset;
};
