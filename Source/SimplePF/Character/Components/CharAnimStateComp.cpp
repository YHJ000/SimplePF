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
		//��/��
		Speed = FVector::DotProduct(Owner->GetVelocity(), Owner->GetActorForwardVector());

		//��/��
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
	//�ܺο��� ������ ó���Ҷ� �ִ� ���� �����ؾ��ϴ� ���
	if (!Owner->HasAuthority())
		return;

	CurAnimState = EAnimState::AnimState_Die;

	//Ŭ��� ��Ƽĳ��Ʈ
	Multicast_Die_Effect();

	// ���������� ȣ��Ʈ Ŭ�� ȭ�� ��� ����
	ForClient_ChangeAnimState();
}

void UCharAnimStateComp::HealthComp_ServerCall_Hit()
{
	//�ܺο��� ������ ó���Ҷ� �ִ� ���� �����ؾ��ϴ� ���
	if (!Owner->HasAuthority())
		return;

	CurAnimState = EAnimState::AnimState_Hit;

	//Ŭ��� ��Ƽĳ��Ʈ(��Ʈ �����϶� ��Ʈ�ϼ��� ����)
	Multicast_Hit_MontageEffect();

	// ���������� ȣ��Ʈ Ŭ�� ȭ�� ��� ����
	ForClient_ChangeAnimState();
}

void UCharAnimStateComp::Multicast_Die_Effect_Implementation()
{
	//����Ʈ�� Ŭ�� �ص� ��
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
	//�������� Ŭ�󿡰� ��Ƽĳ��Ʈ ������ ����

	//Hit�϶� Hit�� ��Ÿ�� �ٽ� �÷�����
	ForClient_ChangeAnimState();

	//����Ʈ�� Ŭ�� �ص� ��
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
	//���� ����
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

	// ���������� ȣ��Ʈ Ŭ�� ȭ�� ��� ����
	ForClient_ChangeAnimState();

	//�Ѿ�/���� Ʈ���̽� ����
	if (auto* WeaponComp = Owner->Get_WeaponComp())
	{
		WeaponComp->Authority_WeaponShootProcess();
	}

	//1���Ŀ� ��Ÿ�� ����
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

			// ���������� ȣ��Ʈ Ŭ�� ȭ�� ��� ����
			ForClient_ChangeAnimState();

		}), 0.4f, false);
}

void UCharAnimStateComp::OnRep_ChangeAnimState()
{
	//Ŭ�� ���ſ�(CurAnimState ������ ����Ǿ�����)

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
