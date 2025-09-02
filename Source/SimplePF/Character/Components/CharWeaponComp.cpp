// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/CharWeaponComp.h"
#include "Character/Weapon/BaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Character/BaseChar.h"
#include "SimplePF.h"
#include "Net/UnrealNetwork.h"
#include "Character/DataAsset/CharWeaponDataAsset.h"
#include "Components/ChildActorComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UserChar.h"

UCharWeaponComp::UCharWeaponComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharWeaponComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharWeaponComp, eCur_WeaponType);
}

void UCharWeaponComp::BeginPlay()
{
	Super::BeginPlay();

	eCur_WeaponType = EWeaponType::WeaponType_HandGun;

	ABaseChar* CharOwner = Cast<ABaseChar>(GetOwner());
	if (CharOwner)
	{
		Owner = CharOwner;

		//���� ������ ���������� ó��
		if (CharOwner->HasAuthority())
		{
			for (int i = (int)EWeaponType::WeaponType_HandGun; i <= (int)EWeaponType::WeaponType_Lifle; ++i)
			{
				EWeaponType WeaponType = static_cast<EWeaponType>(i);
				if (auto* WeaponClass = Map_WeaponClass.Find(WeaponType))
				{
					FActorSpawnParameters Params;
					Params.Owner = CharOwner;
					Params.Instigator = CharOwner;
					Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					UChildActorComponent* ChildActorComp = nullptr;
					if (WeaponType == EWeaponType::WeaponType_HandGun)
						ChildActorComp = CharOwner->Get_ChildActorComp_HandGun();
					else if (WeaponType == EWeaponType::WeaponType_Lifle)
						ChildActorComp = CharOwner->Get_ChildActorComp_Lifle();

					if (nullptr == ChildActorComp)
						continue;

					ChildActorComp->SetChildActorClass(*WeaponClass);

					ABaseWeapon* WeaponActor = Cast<ABaseWeapon>(ChildActorComp->GetChildActor());
					if (nullptr == WeaponActor)
					{
						//�ڵ����� ���ο��� ȣ��ȵ����� �������� ȣ���ؼ� ����
						ChildActorComp->CreateChildActor();
					}

					if (WeaponActor)
					{
						WeaponActor->AttachWeapon(CharOwner, ChildActorComp);

						Map_ChildWeaponComp.Add(WeaponType, ChildActorComp);
					}
				}
			}
			
			ForClient_ChangeWeapon(EWeaponType::WeaponType_Lifle);
		}
		else
		{
			//Ŭ��� ���� �Ϸ�ɶ����� ��ٷȴٰ� Map�� ���
			for (int i = (int)EWeaponType::WeaponType_HandGun; i <= (int)EWeaponType::WeaponType_Lifle; ++i)
			{
				EWeaponType WeaponType = static_cast<EWeaponType>(i);
				if (WeaponType == EWeaponType::WeaponType_HandGun)
				{
					CheckWeaponReplication(WeaponType, CharOwner->Get_ChildActorComp_HandGun());
				}
				else if (WeaponType == EWeaponType::WeaponType_Lifle)
				{
					CheckWeaponReplication(WeaponType, CharOwner->Get_ChildActorComp_Lifle());
				}
			}
		}
	}
}

void UCharWeaponComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

UChildActorComponent* UCharWeaponComp::Find_WeaponComp(EWeaponType WeaponType)
{
	if (auto* ValuePtr = Map_ChildWeaponComp.Find(WeaponType))
	{
		return *ValuePtr;
	}

	return nullptr;
}

void UCharWeaponComp::SetVisibleWeapon(EWeaponType WeaponType, bool bVisible)
{
	if (WeaponType == EWeaponType::WeaponType_HandGun)
	{
		if (AUserChar* Char = Cast<AUserChar>(GetOwner()))
		{
			Char->SetCameraFOV(90.f);
		}
	}

	UChildActorComponent* WeaponComp = Find_WeaponComp(WeaponType);
	if (WeaponComp)
	{
		WeaponComp->SetHiddenInGame(!bVisible);

		if (ABaseWeapon* WeaponActor = Cast<ABaseWeapon>(WeaponComp->GetChildActor()))
		{
			WeaponActor->SetActorHiddenInGame(!bVisible);

			if (USkeletalMeshComponent* Mesh = WeaponActor->FindComponentByClass<USkeletalMeshComponent>())
			{
				Mesh->SetHiddenInGame(!bVisible);

				if (bVisible)
				{
					Mesh->SetVisibility(bVisible, bVisible);
				}
				else
				{
					Mesh->SetVisibility(bVisible);
				}
			}
		}
	}
}

void UCharWeaponComp::CheckWeaponReplication(EWeaponType WeaponType, UChildActorComponent* ChildActorComp)
{
	//Ŭ��� ���� ������ �Ŀ� Map�� ���
	FTimerHandle Handle;

	TWeakObjectPtr<UChildActorComponent> WeakChildComp = ChildActorComp;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this, WeaponType, WeakChildComp]()
		{
			ABaseChar* Char = Cast<ABaseChar>(GetOwner());
			if (Char && WeakChildComp.IsValid())
			{
				ABaseWeapon* WeaponActor = Cast<ABaseWeapon>(WeakChildComp->GetChildActor());
				if (nullptr == WeaponActor)
				{
					//�ڵ����� ���ο��� ȣ��ȵ����� �������� ȣ���ؼ� ����
					WeakChildComp->CreateChildActor();
				}

				if (WeaponActor)
				{
					WeaponActor->AttachWeapon(Char, WeakChildComp.Get());

					Map_ChildWeaponComp.Add(WeaponType, WeakChildComp.Get());
				}
			}

			ForClient_ChangeWeapon(EWeaponType::WeaponType_Lifle);

		}), 1.f, false);
}

void UCharWeaponComp::Authority_WeaponShootProcess()
{
	UChildActorComponent* Cur_WeaponComp = Find_WeaponComp(eCur_WeaponType);
	if (!Cur_WeaponComp)
		return;

	AActor* ChildActor = Cur_WeaponComp->GetChildActor();
	if (!ChildActor)
		return;

	ABaseWeapon* Weapon = Cast<ABaseWeapon>(ChildActor);
	if (!Weapon)
		return;

	switch (eCur_WeaponType)
	{
	case EWeaponType::WeaponType_HandGun:		Weapon->Authority_Shoot_HandGun_SpawnProjectile();	break;
	case EWeaponType::WeaponType_Lifle:			Weapon->Authority_Shoot_Lifle_LineTrace();			break;

	default:
		break;
	}
}

void UCharWeaponComp::Server_ChangeWeapon_Implementation(EWeaponType ChangeWP)
{
	//���� ���� �Լ�
	if (!Owner.IsValid())
		return;

	if (eCur_WeaponType == ChangeWP)
		return;

	UChildActorComponent* Cur_WeaponComp = Find_WeaponComp(eCur_WeaponType);
	UChildActorComponent* Change_WeaponComp = Find_WeaponComp(ChangeWP);

	//������ ���Ⱑ ������ ���� ����
	if (!Change_WeaponComp)
		return;

	EWeaponType PrevWeaponType = eCur_WeaponType;

	//������ ����
	eCur_WeaponType = ChangeWP;

	// ���������� ȣ��Ʈ Ŭ�� ȭ�� ��� ����
	ForClient_ChangeWeapon(PrevWeaponType);
}

void UCharWeaponComp::OnRep_ChangeWeapon(EWeaponType PrevWeaponType)
{
	//���� Ÿ�԰� ���� ���ڷ� �����ϸ� ������ ���� ���� ���� ������ ��

	ForClient_ChangeWeapon(PrevWeaponType);
}

void UCharWeaponComp::ForClient_ChangeWeapon(EWeaponType PrevWeaponType)
{
	//���� ���� ����
	SetVisibleWeapon(PrevWeaponType, false);

	//�ٲ� ���� ����
	SetVisibleWeapon(eCur_WeaponType, true);

	OnDel_ChangeWeapon.Broadcast(eCur_WeaponType);
}