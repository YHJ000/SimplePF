// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefineEnum.h"

#include "BaseWeapon.generated.h"

class ABaseChar;
class UChildActorComponent;
class UCharWeaponDataAsset;
class USkeletalMeshComponent;

UCLASS(Blueprintable)
class SIMPLEPF_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

public:
	ABaseWeapon();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	void AttachWeapon(ABaseChar* OwnerChar, UChildActorComponent* ChildComp);
	void Authority_Shoot_HandGun_SpawnProjectile();
	void Authority_Shoot_Lifle_LineTrace();

public:
	//���� ���ý� �ʿ��� ������
	UPROPERTY(EditDefaultsOnly, Category = "ABaseWeapon")
	TObjectPtr<UCharWeaponDataAsset> DataAsset;

	//����
	UPROPERTY(EditDefaultsOnly, Category = "ABaseWeapon")
	TObjectPtr<USkeletalMeshComponent> SKMeshComp;
};
