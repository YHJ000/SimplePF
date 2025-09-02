// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseChar.h"
#include "Components/CapsuleComponent.h"
#include "Character/Components/CharAnimStateComp.h"
#include "Character/Components/CharWeaponComp.h"
#include "Character/Components/CharHealthComp.h"
#include "Components/ChildActorComponent.h"
#include "Actor/Projectile.h"
#include "Engine/ActorChannel.h"

// Sets default values
ABaseChar::ABaseChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	AnimFSMComp = CreateDefaultSubobject<UCharAnimStateComp>(TEXT("AnimFSMComp"));
	WeaponComp = CreateDefaultSubobject<UCharWeaponComp>(TEXT("WeaponComp"));
	HealthComp = CreateDefaultSubobject<UCharHealthComp>(TEXT("HealthComp"));

	ChildActorComp_HandGun = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildActorComp_HandGun"));
	ChildActorComp_Lifle = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildActorComp_Lifle"));

	AnimFSMComp->SetIsReplicated(true);
	WeaponComp->SetIsReplicated(true);
	HealthComp->SetIsReplicated(true);

	AnimFSMComp->SetNetAddressable();
	WeaponComp->SetNetAddressable();
	HealthComp->SetNetAddressable();

	UCapsuleComponent* CapsulComp = GetCapsuleComponent();
	if (CapsulComp)
	{
		CapsulComp->SetGenerateOverlapEvents(true);
		CapsulComp->SetCollisionProfileName(TEXT("PF_Char"));
	}

	OnActorBeginOverlap.AddDynamic(this, &ABaseChar::OnBeginOverlap);
}

void ABaseChar::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseChar::OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	//���� �Ǵ�
	if (!HasAuthority())
		return;

	if (!HealthComp.Get())
		return;

	//�ڱ� �ڽ��� �ƴϸ� ����
	if (this != OverlappedActor)
		return;

	//�ڱ� �ڽ��̴ϱ� �÷��� ���� ó��(���� Ÿ�� ����)
	if (OtherActor->ActorHasTag(TEXT("Projectile")))
	{
		if (AProjectile* Projectile = Cast<AProjectile>(OtherActor))
		{
			//�Ѿ��� �߻��� ����� �ڱ� �ڽ��̸� ����
			if (this == OtherActor->GetOwner())
				return;

			//HandGun
			HealthComp->Server_OnHit(Projectile->Damage);
		}
	}
}

void ABaseChar::Authority_OnHitLineTrace(float Damage)
{
	//���� �Ǵ�
	if (!HasAuthority())
		return;

	if (!HealthComp.Get())
		return;

	//LineTrace�� Lifle
	HealthComp->Server_OnHit(Damage);
}