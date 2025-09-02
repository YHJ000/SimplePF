// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Weapon/BaseWeapon.h"
#include "Character/BaseChar.h"
#include "Character/DataAsset/CharWeaponDataAsset.h"
#include "Actor/Projectile.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "SimplePF.h"
#include "Engine/CollisionProfile.h"

ABaseWeapon::ABaseWeapon()
{
	Tags.Add(TEXT("Weapon"));

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SKMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (SKMeshComp.Get() && DataAsset.Get())
	{
		SKMeshComp->SetSkeletalMesh(DataAsset->SkeletalMesh.Get());
		SKMeshComp->SetRelativeTransform(DataAsset->RH_RelativeTransform);
	}
}

void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseWeapon::AttachWeapon(ABaseChar* OwnerChar, UChildActorComponent* ChildComp)
{
	if (nullptr == OwnerChar || nullptr == ChildComp)
		return;

	if (!SKMeshComp.Get() || !DataAsset.Get())
		return;

	if (!SKMeshComp->DoesSocketExist(DataAsset->GripSocket))
		return;

	SetOwner(OwnerChar);

	//ChildActorComp자체를 해당 소켓에 붙이고, Actor은 따라가게
	ChildComp->AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, DataAsset->CharAttachSocket);

	SKMeshComp->SetRelativeTransform(DataAsset->RH_RelativeTransform);
}

void ABaseWeapon::Authority_Shoot_HandGun_SpawnProjectile()
{
	ABaseChar* MyOwner = Cast<ABaseChar>(GetOwner());
	if (!MyOwner || !SKMeshComp.Get() || !DataAsset.Get() 
		|| nullptr == DataAsset->ClassWeapon || nullptr == DataAsset->ClassProjectile)
		return;

	UWorld* World = GetWorld();
	AController* Controller = MyOwner->GetController();
	if (!World || !Controller)
		return;

	if (!SKMeshComp->DoesSocketExist(DataAsset->FireProjectileSocket))
		return;

	//컨트롤러가 보는 카메라
	FVector CamPos;
	FRotator CamDir;
	Controller->GetPlayerViewPoint(CamPos, CamDir);

	//대충 먼 곳
	FVector LineTraceEndPos = CamPos + CamDir.Vector() * 10000.f;

	//내 컨트롤러에서 라인트레이스
	FHitResult HitResult;
	ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(MyOwner->Get_Cache_TraceType_Shoot().GetValue());

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FVector HitEndPos;
	if (World->LineTraceSingleByChannel(HitResult, CamPos, LineTraceEndPos, CollisionChannel, QueryParams))
	{
		HitEndPos = HitResult.ImpactPoint;
	}
	else
	{
		//안맞으면 대충 먼 곳 위치
		HitEndPos = LineTraceEndPos;
	}


	//총구 소켓 위치
	FVector FirePos = SKMeshComp->GetSocketLocation(DataAsset->FireProjectileSocket);
	float MinConvergeDist = 150.f;

	// HitEndPos가 카메라 앞쪽인지 검사해서 거리 스칼라값 보정
	FVector CamFwd = CamDir.Vector();
	float tForward = FVector::DotProduct(HitEndPos - CamPos, CamFwd);
	if (tForward < 0.f)
	{
		// 뒤쪽이면 앞쪽 최소거리로 강제
		tForward = MinConvergeDist;
	}
	else if (tForward < MinConvergeDist)
	{
		// 앞쪽인데 최소 거리보다 가까움(최소 거리 보장)
		tForward = MinConvergeDist;
	}

	//보정된 거리 스칼라값만큼 방향 다시줘서 HitEndPos 보정
	HitEndPos = CamPos + CamFwd * tForward;

	// 총구가 최소 거리 보정 위치 보다 앞일 경우(상황 : 카메라 < 보정 위치 < 총구)
	if (FVector::DotProduct(HitEndPos - FirePos, CamFwd) <= 0.f)
	{
		HitEndPos = FirePos + CamFwd * MinConvergeDist;
	}

	FVector ShootDir = (HitEndPos - FirePos).GetSafeNormal();

	//총알에 오너 셋팅(파람 넣으면 자동 셋팅해줌)
	FActorSpawnParameters Params;
	Params.Owner = MyOwner;
	Params.Instigator = MyOwner;

	AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(DataAsset->ClassProjectile.Get(), FirePos, ShootDir.Rotation(), Params);
	if (Projectile)
	{
		Projectile->SetSpawnProcess(DataAsset->Damage);
	}

	//총격 사운드(맞던 말던 쏜거니까)
	if (DataAsset->ShootSound.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DataAsset->ShootSound, Owner->GetActorLocation());
	}
}

void ABaseWeapon::Authority_Shoot_Lifle_LineTrace()
{
	ABaseChar* MyOwner = Cast<ABaseChar>(GetOwner());
	if (!MyOwner || !SKMeshComp.Get() || !DataAsset.Get() || nullptr == DataAsset->ClassWeapon)
		return;

	UWorld* World = GetWorld();
	AController* Controller = MyOwner->GetController();
	if (!World || !Controller)
		return;

	if (!SKMeshComp->DoesSocketExist(DataAsset->FireProjectileSocket))
		return;

	//컨트롤러가 보는 카메라
	FVector CamPos;
	FRotator CamDir;
	Controller->GetPlayerViewPoint(CamPos, CamDir);

	//대충 먼 곳
	FVector LineTraceEndPos = CamPos + CamDir.Vector() * 10000.f;

	//내 컨트롤러에서 라인트레이스
	FHitResult HitResult;
	ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(MyOwner->Get_Cache_TraceType_Shoot().GetValue());

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FVector HitEndPos;
	if (World->LineTraceSingleByChannel(HitResult, CamPos, LineTraceEndPos, CollisionChannel, QueryParams))
	{
		HitEndPos = HitResult.ImpactPoint;

		if (ABaseChar* HitChar = Cast<ABaseChar>(HitResult.GetActor()))
		{
			HitChar->Authority_OnHitLineTrace(DataAsset->Damage);
		}
	}

	//총격 사운드(맞던 말던 쏜거니까)
	if (DataAsset->ShootSound.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DataAsset->ShootSound, Owner->GetActorLocation());
	}
}