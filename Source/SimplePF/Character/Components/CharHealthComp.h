// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"

#include "CharHealthComp.generated.h"

class ABaseChar;
class UCharAnimStateComp;

DECLARE_MULTICAST_DELEGATE_TwoParams(FMyCharHP, float /*HP*/, float /*MaxHP*/);
DECLARE_MULTICAST_DELEGATE(FMyCharDie);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIMPLEPF_API UCharHealthComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharHealthComp();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

public:
	FORCEINLINE bool Get_IsDie() { return IsDie; }
	FORCEINLINE float Get_HP() { return HP; }
	FORCEINLINE float Get_MaxHP() { return MaxHP; }

public:
	UFUNCTION(Server, Reliable)
	void Server_OnHit(float Damage);

	UFUNCTION()
	void OnRep_HP();
	void ForClient_HP();

	UFUNCTION()
	void OnRep_Die();
	void ForClient_Die();

public:
	FMyCharHP OnDel_MyCharHP;
	FMyCharDie OnDel_MyCharDie;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Die, BlueprintReadOnly, Category = "HealthComp")
	bool IsDie = false;

	UPROPERTY(ReplicatedUsing = OnRep_HP, BlueprintReadOnly, Category = "HealthComp")
	float HP = 200;

	const float MaxHP = 200.f;

	UPROPERTY(transient, BlueprintReadOnly, Category = "HealthComp")
	TWeakObjectPtr<ABaseChar> Owner;

	UPROPERTY(transient, BlueprintReadOnly, Category = "HealthComp")
	TWeakObjectPtr<UCharAnimStateComp> AnimFSMComp;
};
