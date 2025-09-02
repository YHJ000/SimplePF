// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DefineEnum.h"

#include "BaseChar.generated.h"

class UCharAnimStateComp;
class UCharWeaponComp;
class UCharHealthComp;
class UChildActorComponent;

UCLASS(Blueprintable)
class SIMPLEPF_API ABaseChar : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseChar();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE UCharAnimStateComp* Get_AnimFSMComp() const { return AnimFSMComp.Get() ? AnimFSMComp.Get() : nullptr; }
	FORCEINLINE UCharWeaponComp* Get_WeaponComp() const { return WeaponComp.Get() ? WeaponComp.Get() : nullptr; }
	FORCEINLINE UCharHealthComp* Get_HealthComp() const { return HealthComp.Get() ? HealthComp.Get() : nullptr; }
	FORCEINLINE UChildActorComponent* Get_ChildActorComp_HandGun() const { return ChildActorComp_HandGun.Get() ? ChildActorComp_HandGun.Get() : nullptr; }
	FORCEINLINE UChildActorComponent* Get_ChildActorComp_Lifle() const { return ChildActorComp_Lifle.Get() ? ChildActorComp_Lifle.Get() : nullptr; }

public:
	UFUNCTION()
	virtual void OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	void Authority_OnHitLineTrace(float Damage);
	FORCEINLINE TEnumAsByte<ETraceTypeQuery> Get_Cache_TraceType_Shoot() { return Cache_TraceType_Shoot; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseChar")
	TObjectPtr<UCharAnimStateComp> AnimFSMComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseChar")
	TObjectPtr<UCharWeaponComp> WeaponComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseChar")
	TObjectPtr<UCharHealthComp> HealthComp;

	//항상 존재하는 무기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseChar")
	TObjectPtr<UChildActorComponent> ChildActorComp_HandGun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseChar")
	TObjectPtr<UChildActorComponent> ChildActorComp_Lifle;

	UPROPERTY(EditDefaultsOnly, Category = "BaseChar")
	TEnumAsByte<ETraceTypeQuery>	Cache_TraceType_Shoot;
};
