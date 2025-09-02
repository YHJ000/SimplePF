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

	//ChildActorComp��ü�� �ش� ���Ͽ� ���̰�, Actor�� ���󰡰�
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

	//��Ʈ�ѷ��� ���� ī�޶�
	FVector CamPos;
	FRotator CamDir;
	Controller->GetPlayerViewPoint(CamPos, CamDir);

	//���� �� ��
	FVector LineTraceEndPos = CamPos + CamDir.Vector() * 10000.f;

	//�� ��Ʈ�ѷ����� ����Ʈ���̽�
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
		//�ȸ����� ���� �� �� ��ġ
		HitEndPos = LineTraceEndPos;
	}


	//�ѱ� ���� ��ġ
	FVector FirePos = SKMeshComp->GetSocketLocation(DataAsset->FireProjectileSocket);
	float MinConvergeDist = 150.f;

	// HitEndPos�� ī�޶� �������� �˻��ؼ� �Ÿ� ��Į�� ����
	FVector CamFwd = CamDir.Vector();
	float tForward = FVector::DotProduct(HitEndPos - CamPos, CamFwd);
	if (tForward < 0.f)
	{
		// �����̸� ���� �ּҰŸ��� ����
		tForward = MinConvergeDist;
	}
	else if (tForward < MinConvergeDist)
	{
		// �����ε� �ּ� �Ÿ����� �����(�ּ� �Ÿ� ����)
		tForward = MinConvergeDist;
	}

	//������ �Ÿ� ��Į�󰪸�ŭ ���� �ٽ��༭ HitEndPos ����
	HitEndPos = CamPos + CamFwd * tForward;

	// �ѱ��� �ּ� �Ÿ� ���� ��ġ ���� ���� ���(��Ȳ : ī�޶� < ���� ��ġ < �ѱ�)
	if (FVector::DotProduct(HitEndPos - FirePos, CamFwd) <= 0.f)
	{
		HitEndPos = FirePos + CamFwd * MinConvergeDist;
	}

	FVector ShootDir = (HitEndPos - FirePos).GetSafeNormal();

	//�Ѿ˿� ���� ����(�Ķ� ������ �ڵ� ��������)
	FActorSpawnParameters Params;
	Params.Owner = MyOwner;
	Params.Instigator = MyOwner;

	AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(DataAsset->ClassProjectile.Get(), FirePos, ShootDir.Rotation(), Params);
	if (Projectile)
	{
		Projectile->SetSpawnProcess(DataAsset->Damage);
	}

	//�Ѱ� ����(�´� ���� ��Ŵϱ�)
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

	//��Ʈ�ѷ��� ���� ī�޶�
	FVector CamPos;
	FRotator CamDir;
	Controller->GetPlayerViewPoint(CamPos, CamDir);

	//���� �� ��
	FVector LineTraceEndPos = CamPos + CamDir.Vector() * 10000.f;

	//�� ��Ʈ�ѷ����� ����Ʈ���̽�
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

	//�Ѱ� ����(�´� ���� ��Ŵϱ�)
	if (DataAsset->ShootSound.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DataAsset->ShootSound, Owner->GetActorLocation());
	}
}