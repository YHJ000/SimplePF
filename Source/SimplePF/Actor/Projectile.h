// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Projectile.generated.h"

class ABaseChar;
class UBoxComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class SIMPLEPF_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void SetSpawnProcess(float SetDamage);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AProjectile")
	float Speed = 2000.f;

	float Damage = 50.f;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AProjectile")
	TObjectPtr<UBoxComponent> CollisionComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AProjectile")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AProjectile")
	TObjectPtr<UProjectileMovementComponent> MovementComp;
};
