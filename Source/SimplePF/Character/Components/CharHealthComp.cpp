// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/CharHealthComp.h"
#include "Net/UnrealNetwork.h"
#include "Character/BaseChar.h"
#include "Character/Components/CharAnimStateComp.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UserChar.h"

UCharHealthComp::UCharHealthComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharHealthComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharHealthComp, HP);
	DOREPLIFETIME(UCharHealthComp, IsDie);
}

void UCharHealthComp::BeginPlay()
{
	Super::BeginPlay();

	if (ABaseChar* CharOwner = Cast<ABaseChar>(GetOwner()))
	{
		Owner = CharOwner;
		if (USkeletalMeshComponent* Mesh = CharOwner->GetMesh())
		{
			AnimFSMComp = Cast<UCharAnimStateComp>(CharOwner->Get_AnimFSMComp());
		}
	}
}

void UCharHealthComp::Server_OnHit_Implementation(float Damage)
{
	//�̵�/�Է�/�ݸ��� ��Ȱ��ȭ
	//����/���/������ �÷��� ���� ����

	if (!Owner.IsValid() || !AnimFSMComp.IsValid())
		return;

	if (IsDie)
		return;

	//HP ����
	HP = FMath::Clamp(HP - Damage, 0.f, MaxHP);

	// ���������� ȣ��Ʈ Ŭ�� ȭ�� ��� ����
	ForClient_HP();

	if (HP <= 0)
	{
		//���� ó��
		IsDie = true;

		// ���������� ȣ��Ʈ Ŭ�� ȭ�� ��� ����
		ForClient_Die();

		//FSM ���� ���� ���� ó��
		AnimFSMComp->HealthComp_ServerCall_Die();

		//������ ������ �Է��� ����
	}
	else
	{
		//FSM ���� ���� ���� ó��
		AnimFSMComp->HealthComp_ServerCall_Hit();
	}
}

void UCharHealthComp::OnRep_HP()
{
	//Ŭ�� ���ſ�
	ForClient_HP();
}

void UCharHealthComp::ForClient_HP()
{
	if (!Owner.IsValid())
		return;

	//����
	OnDel_MyCharHP.Broadcast(HP, MaxHP);
}

void UCharHealthComp::OnRep_Die()
{
	//Ŭ�� ���ſ�
	ForClient_Die();
}

void UCharHealthComp::ForClient_Die()
{
	if (!Owner.IsValid() || !Owner->GetController())
		return;

	//Look, �Է� ����
	Owner->GetController()->SetIgnoreLookInput(true);
	Owner->GetController()->SetIgnoreMoveInput(true);

	//����
	OnDel_MyCharDie.Broadcast();
}
