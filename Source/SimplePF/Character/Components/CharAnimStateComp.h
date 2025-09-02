// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DefineEnum.h"

#include "CharAnimStateComp.generated.h"

//애니메이션 FSM용 상태 변수 관리

class ABaseChar;
class UBaseCharAnimInstance;
class UParticleSystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIMPLEPF_API UCharAnimStateComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharAnimStateComp();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void HealthComp_ServerCall_Die();
	void HealthComp_ServerCall_Hit();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Die_Effect();

	UFUNCTION(NetMulticast, UnReliable)
	void Multicast_Hit_MontageEffect();

	UFUNCTION(Server, Reliable)
	void Server_Shoot();

	UFUNCTION()
	void OnRep_ChangeAnimState();
	void ForClient_ChangeAnimState();

protected:
	UPROPERTY(ReplicatedUsing=OnRep_ChangeAnimState, BlueprintReadOnly, Category = "AnimFSMData")
	EAnimState CurAnimState = EAnimState::AnimState_Idle;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AnimFSMData")
	bool IsCoolTimeShoot = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AnimFSMData")
	float Speed = 0.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AnimFSMData")
	float Right = 0.f;

protected:
	UPROPERTY(transient, BlueprintReadOnly, Category = "AnimFSMData")
	TWeakObjectPtr<ABaseChar> Owner;

	//AnimInstance 갱신용
	UPROPERTY(transient, BlueprintReadOnly, Category = "AnimFSMData")
	TWeakObjectPtr<UBaseCharAnimInstance> OwnerAnimInstance;

	//Hit 파티클
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimFSMData")
	TObjectPtr<UParticleSystem> HitParticle;
};
