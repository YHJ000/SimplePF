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
	//이동/입력/콜리전 비활성화
	//점수/드랍/리스폰 플레이 상태 변경

	if (!Owner.IsValid() || !AnimFSMComp.IsValid())
		return;

	if (IsDie)
		return;

	//HP 감소
	HP = FMath::Clamp(HP - Damage, 0.f, MaxHP);

	// 리슨서버의 호스트 클라 화면 즉시 보정
	ForClient_HP();

	if (HP <= 0)
	{
		//죽음 처리
		IsDie = true;

		// 리슨서버의 호스트 클라 화면 즉시 보정
		ForClient_Die();

		//FSM 상태 변수 복제 처리
		AnimFSMComp->HealthComp_ServerCall_Die();

		//서버는 어차피 입력이 없음
	}
	else
	{
		//FSM 상태 변수 복제 처리
		AnimFSMComp->HealthComp_ServerCall_Hit();
	}
}

void UCharHealthComp::OnRep_HP()
{
	//클라 수신용
	ForClient_HP();
}

void UCharHealthComp::ForClient_HP()
{
	if (!Owner.IsValid())
		return;

	//연출
	OnDel_MyCharHP.Broadcast(HP, MaxHP);
}

void UCharHealthComp::OnRep_Die()
{
	//클라 수신용
	ForClient_Die();
}

void UCharHealthComp::ForClient_Die()
{
	if (!Owner.IsValid() || !Owner->GetController())
		return;

	//Look, 입력 막음
	Owner->GetController()->SetIgnoreLookInput(true);
	Owner->GetController()->SetIgnoreMoveInput(true);

	//연출
	OnDel_MyCharDie.Broadcast();
}
