// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"

#include "HandGunScopeWidget.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLEPF_API UHandGunScopeWidget : public UBaseUserWidget
{
	GENERATED_BODY()

protected:
	virtual void HandleVisibilityChanged(ESlateVisibility InVisibility);
};
