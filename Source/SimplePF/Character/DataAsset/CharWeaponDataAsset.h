// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CharWeaponDataAsset.generated.h"

class ABaseWeapon;
class AProjectile;
class USkeletalMesh;

UCLASS(BlueprintType)
class SIMPLEPF_API UCharWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "UCharWeaponDataAsset")
	TSubclassOf<ABaseWeapon> ClassWeapon;

	//�߻�ü
	UPROPERTY(EditDefaultsOnly, Category = "UCharWeaponDataAsset")
	TSubclassOf<AProjectile> ClassProjectile;

	UPROPERTY(EditDefaultsOnly, Category = "UCharWeaponDataAsset")
	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = "UCharWeaponDataAsset")
	TObjectPtr<USoundBase> ShootSound;

	//���� �������� TSoftPtr�� �񵿱�ε�� �����ϱ�
	UPROPERTY(EditDefaultsOnly, Category = "UCharWeaponDataAsset")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, Category = "UCharWeaponDataAsset")
	FName FireProjectileSocket = "Fire_Projectile";

	UPROPERTY(EditDefaultsOnly, Category = "UCharWeaponDataAsset")
	FName CharAttachSocket = "hand_r";

	UPROPERTY(EditDefaultsOnly, Category = "UCharWeaponDataAsset")
	FName GripSocket = "RH_Grip";

	UPROPERTY(EditDefaultsOnly, Category = "UCharWeaponDataAsset")
	FTransform RH_RelativeTransform;
};
