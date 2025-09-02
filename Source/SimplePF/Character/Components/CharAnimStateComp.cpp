// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/CharAnimStateComp.h"
#include "Character/BaseChar.h"
#include "Net/UnrealNetwork.h"
#include "Character/AnimInstance/BaseCharAnimInstance.h"
#include "TimerManager.h"
#include "Character/Components/CharHealthComp.h"
#include "Character/Components/CharWeaponComp.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "SimplePF.h"

UCharAnimStateComp::UCharAnimStateComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharAnimStateComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCharAnimStateComp, CurAnimState, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(UCharAnimStateComp, IsCoolTimeShoot);
	DOREPLIFETIME(UCharAnimStateComp, Speed);
	DOREPLIFETIME(UCharAnimStateComp, Right);
}

void UCharAnimStateComp::BeginPlay()
{
	Super::BeginPlay();

	if (ABaseChar* CharOwner = Cast<ABaseChar>(GetOwner()))
	{
		Owner = CharOwner;

		if (USkeletalMeshComponent* Mesh = CharOwner->GetMesh())
		{
			OwnerAnimInstance = Cast<UBaseCharAnimInstance>(Mesh->GetAnimInstance());
		}
	}
}

void UCharAnimStateComp::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Owner.IsValid())
	{
		//앞/뒤
		Speed = FVector::DotProduct(Owner->GetVelocity(), Owner->GetActorForwardVector());

		//우/좌
		Right = FVector::DotProduct(Owner->GetVelocity(), Owner->GetActorRightVector());

		if (OwnerAnimInstance.IsValid())
		{
			OwnerAnimInstance->Update_Speed(Speed);
			OwnerAnimInstance->Update_Right(Right);
		}
	}
}

void UCharAnimStateComp::HealthComp_ServerCall_Die()
{	
	//외부에서 서버가 처리할때 애니 상태 변경해야하는 경우
	if (!Owner->HasAuthority())
		return;

	CurAnimState = EAnimState::AnimState_Die;

	//클라들 멀티캐스트
	Multicast_Die_Effect();

	// 리슨서버의 호스트 클라 화면 즉시 보정
	ForClient_ChangeAnimState();
}

void UCharAnimStateComp::HealthComp_ServerCall_Hit()
{
	//외부에서 서버가 처리할때 애니 상태 변경해야하는 경우
	if (!Owner->HasAuthority())
		return;

	CurAnimState = EAnimState::AnimState_Hit;

	//클라들 멀티캐스트(히트 상태일때 히트일수도 있음)
	Multicast_Hit_MontageEffect();

	// 리슨서버의 호스트 클라 화면 즉시 보정
	ForClient_ChangeAnimState();
}

void UCharAnimStateComp::Multicast_Die_Effect_Implementation()
{
	//이펙트는 클라만 해도 됨
	if (Owner.IsValid())
	{
		if (HitParticle.Get())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle.Get(), Owner->GetTransform());
		}
	}
}

void UCharAnimStateComp::Multicast_Hit_MontageEffect_Implementation()
{
	//서버에서 클라에게 멀티캐스트 보내면 받음

	//Hit일때 Hit면 몽타주 다시 플레이함
	ForClient_ChangeAnimState();

	//이펙트는 클라만 해도 됨
	if (Owner.IsValid())
	{
		if (HitParticle.Get())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle.Get(), Owner->GetTransform());
		}
	}
}

void UCharAnimStateComp::Server_Shoot_Implementation()
{
	//서버 전용
	if (!Owner.IsValid())
		return;

	if (!Owner->HasAuthority())
		return;

	UCharHealthComp* HealthComp = Owner->Get_HealthComp();
	if (!HealthComp)
		return;

	if (HealthComp->Get_IsDie())
		return;

	if (CurAnimState == EAnimState::AnimState_Shoot
		|| CurAnimState == EAnimState::AnimState_Die)
		return;

	if (IsCoolTimeShoot)
		return;

	IsCoolTimeShoot = true;
	CurAnimState = EAnimState::AnimState_Shoot;

	// 리슨서버의 호스트 클라 화면 즉시 보정
	ForClient_ChangeAnimState();

	//총알/라인 트레이스 생성
	if (auto* WeaponComp = Owner->Get_WeaponComp())
	{
		WeaponComp->Authority_WeaponShootProcess();
	}

	//1초후에 쿨타임 해제
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this]() 
		{
			IsCoolTimeShoot = false;

			if (CurAnimState == EAnimState::AnimState_Die)
				return;

			if (Owner->GetVelocity().Length() > 0)
			{
				CurAnimState = EAnimState::AnimState_Move;
			}
			else
			{
				CurAnimState = EAnimState::AnimState_Idle;
			}

			// 리슨서버의 호스트 클라 화면 즉시 보정
			ForClient_ChangeAnimState();

		}), 0.4f, false);
}

void UCharAnimStateComp::OnRep_ChangeAnimState()
{
	//클라 수신용(CurAnimState 복제로 변경되어있음)

	if (Owner.IsValid())
	{
		UE_LOG(LogHJ, Log, TEXT("[%s] : OnRep_ChangeAnimState = %s"), *Owner->GetName(), *UEnum::GetValueAsString(CurAnimState));
	}

	if (OwnerAnimInstance.IsValid())
	{
		OwnerAnimInstance->Update_AnimState(CurAnimState);
	}
}

void UCharAnimStateComp::ForClient_ChangeAnimState()
{
	if (OwnerAnimInstance.IsValid())
	{
		OwnerAnimInstance->Update_AnimState(CurAnimState);
	}
}
