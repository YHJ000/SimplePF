// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DefineEnum.h"

#include "BaseUserWidget.generated.h"

class SWidget;
class UUIManagerSubsystem;

UCLASS()
class SIMPLEPF_API UBaseUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
	friend class UUIManagerSubsystem;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	void Bind_HandleVisibilityChanged(ESlateVisibility InVisibility);
	virtual void HandleVisibilityChanged(ESlateVisibility InVisibility);

public:
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	EWidgetName WidgetName;
	TWeakPtr<SWidget> ViewportHandle;
};
