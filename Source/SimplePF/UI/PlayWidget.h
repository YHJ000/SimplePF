// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "PlayWidget.generated.h"

class AUserChar;

UENUM()
enum EPlayWidgetState : uint8
{
	EPlayWidgetState_Alive,
	EPlayWidgetState_Hit,
	EPlayWidgetState_Die,
};

class UButton;

UCLASS()
class SIMPLEPF_API UPlayWidget : public UBaseUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void HandleVisibilityChanged(ESlateVisibility InVisibility) override;

public:
	UFUNCTION(BlueprintCallable, Category = "PlayWidget")
	void QuitListenServer();

private:
	void SetOverlay(EPlayWidgetState WidgetState);

	void UpdateHP(float HP, float MaxHP);
	void UpdateEquipWeapon(EWeaponType EquipWeapon);
	void OnDie();

protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<UUIManagerSubsystem> UIManager;

	UPROPERTY(Transient)
	TWeakObjectPtr<AUserChar> MyChar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UOverlay> Overlay_Die;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UOverlay> Overlay_Hit;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UProgressBar> ProgressBar_HP;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UTextBlock> TextBlock_HP_Value;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UImage> HandGun_Equip;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UImage> Lifle_Equip;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> Button_Quit;

private:
	EPlayWidgetState eWidgetState = EPlayWidgetState_Alive;
};
