// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DefineEnum.generated.h"

UENUM(BlueprintType)
enum class ETableName : uint8
{
	TableName_LoadTable,
	TableName_Widget,
	TableName_Texture,
	TableName_Actor
};

UENUM(BlueprintType)
enum class EWeaponType : uint8 
{
	WeaponType_HandGun,
	WeaponType_Lifle,
};

UENUM(BlueprintType)
enum class EInputAction : uint8 
{
	InputAction_Click,
	InputAction_Num1,
	InputAction_Num2
};

UENUM(BlueprintType)
enum class EAnimState : uint8
{
	AnimState_Idle,
	AnimState_Move,
	AnimState_Shoot,
	AnimState_Hit,
	AnimState_Die
};

UENUM(BlueprintType)
enum class EWidgetName : uint8
{
	WBP_Loading,
	WBP_Start,
	WBP_Play,
	WBP_HandGunScope,
	WBP_LifleScope
};

namespace EnumUtil
{
	template <typename T>
	FORCEINLINE int64 GetValueByName(FName Name)
	{
		UEnum* CastEnum = StaticEnum<T>();
		if (nullptr != CastEnum)
		{
			return CastEnum->GetValueByName(Name);
		}

		return INDEX_NONE;
	}

	template <typename T>
	FORCEINLINE FName GetNameByValue(int64 EnumValue)
	{
		UEnum* CastEnum = StaticEnum<T>();
		if (nullptr != CastEnum)
		{
			FString Name = CastEnum->GetNameStringByValue(EnumValue);
			if (!Name.IsEmpty())
			{
				return FName(*Name);
			}
		}

		return FName();
	}
}