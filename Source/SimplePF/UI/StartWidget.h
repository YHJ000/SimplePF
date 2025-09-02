// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"

#include "StartWidget.generated.h"

class UButton;

UCLASS()
class SIMPLEPF_API UStartWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void HandleVisibilityChanged(ESlateVisibility InVisibility) override;

public:
	UFUNCTION(BlueprintCallable, Category = "StartWidget")
	void HostListenServer();

	UFUNCTION(BlueprintCallable, Category = "StartWidget")
	void JoinListenServer();

private:
	void SetButtonsEnable(bool bEnable);

public:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> Button_Host;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> Button_Join;
};
