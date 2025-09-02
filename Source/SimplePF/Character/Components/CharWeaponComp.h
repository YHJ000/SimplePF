// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DefineEnum.h"

#include "CharWeaponComp.generated.h"

class USceneComponent;
class ABaseWeapon;
class UChildActorComponent;
class ABaseChar;

DECLARE_MULTICAST_DELEGATE_OneParam(FMyCharChangeWeapon, EWeaponType);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIMPLEPF_API UCharWeaponComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharWeaponComp();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	FORCEINLINE	EWeaponType GetCurWeaponType() {	return eCur_WeaponType;	}
	void Authority_WeaponShootProcess();

public:
	UFUNCTION(Server, Reliable)
	void Server_ChangeWeapon(EWeaponType ChangeWP);

	UFUNCTION()
	void OnRep_ChangeWeapon(EWeaponType PrevWeaponType);

	void ForClient_ChangeWeapon(EWeaponType ChangeWP);

	FMyCharChangeWeapon OnDel_ChangeWeapon;

	UPROPERTY(ReplicatedUsing= OnRep_ChangeWeapon, BlueprintReadOnly, Category = "CharWeaponComp", meta = (AllowPrivateAccess = true))
	EWeaponType eCur_WeaponType = EWeaponType::WeaponType_HandGun;

private:
	UChildActorComponent* Find_WeaponComp(EWeaponType WeaponType);
	void SetVisibleWeapon(EWeaponType WeaponType, bool bVisible);
	void CheckWeaponReplication(EWeaponType WeaponType, UChildActorComponent* ChildActorComp);

private:
	//캐릭터마다 항상 존재하는 지정 무기(HandGun, Lifle)
	UPROPERTY(EditDefaultsOnly, Category = "CharWeaponComp")

	TMap<EWeaponType, TSubclassOf<ABaseWeapon>>	Map_WeaponClass;

	//스폰된 보유 무기 액터들
	UPROPERTY(EditDefaultsOnly, Category = "CharWeaponComp")
	TMap<EWeaponType, TObjectPtr<UChildActorComponent>>	Map_ChildWeaponComp;

	UPROPERTY(transient, BlueprintReadOnly, Category = "CharWeaponComp", meta =(AllowPrivateAccess))
	TWeakObjectPtr<ABaseChar> Owner;
};
