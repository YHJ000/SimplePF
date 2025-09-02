// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "RowWidget.generated.h"

class UUserWidget;

USTRUCT(BlueprintType)
struct FRowWidget : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	TSoftClassPtr<UUserWidget> ClassUI;
};
