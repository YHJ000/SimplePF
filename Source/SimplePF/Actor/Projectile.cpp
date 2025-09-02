// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Projectile.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/BaseChar.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SimplePF.h"

// Sets default values
AProjectile::AProjectile()
{
	Tags.Add(TEXT("Projectile"));

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	InitialLifeSpan = 5.f;

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));

	SetRootComponent(CollisionComp);
	CollisionComp->SetBoxExtent(FVector(6.f, 3.f, 3.f));
	CollisionComp->SetCollisionProfileName(TEXT("PF_Projectile"));

	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -3.f));
	MeshComp->SetRelativeScale3D(FVector(0.1f, 0.05f, 0.05f));
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::SetSpawnProcess(float SetDamage)
{
	Damage = SetDamage;

	if (!MovementComp.Get() || !MeshComp.Get())
		return;

	//BaseChar
	if (ABaseChar* MyOwner = Cast<ABaseChar>(GetOwner()))
	{
		MovementComp->SetUpdatedComponent(CollisionComp);
		MovementComp->bRotationFollowsVelocity = true;
		MovementComp->InitialSpeed = Speed;
		MovementComp->MaxSpeed = Speed;
		MovementComp->ProjectileGravityScale = 0.f;
		MovementComp->bShouldBounce = false;	//벽, 바닥 튕김 방지
		MovementComp->bInterpMovement = true;
		MovementComp->bInitialVelocityInLocalSpace = false; // 월드 방향으로 속도 넣기

		MovementComp->Velocity = GetActorRotation().Vector() * MovementComp->InitialSpeed;

		//소유자 무시
		if (CollisionComp.Get())
		{
			CollisionComp->IgnoreActorWhenMoving(MyOwner, true);
		}
	}
}

