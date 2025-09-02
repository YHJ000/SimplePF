// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/DefineEnum.h"

#include "RowLoadTable.generated.h"

USTRUCT(BlueprintType)
struct FRowLoadTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	ETableName TableName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	bool NeedLoadOrder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	TSoftObjectPtr<UDataTable> ObjectTable;
};
