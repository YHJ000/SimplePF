// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ProjectileTriggerBox.h"

void AProjectileTriggerBox::BeginPlay()
{
	Super::BeginPlay();

	OnActorEndOverlap.AddDynamic(this, &AProjectileTriggerBox::OnEndOverlap);
}

void AProjectileTriggerBox::OnEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	//�浹 �Ǵ� �� ���� ������ ����������
	if (!HasAuthority())
		return;

	if (!OtherActor)
		return;

	//AProjectile
	if (OtherActor->ActorHasTag(TEXT("Projectile")))
	{
		OtherActor->Destroy();
	}
}