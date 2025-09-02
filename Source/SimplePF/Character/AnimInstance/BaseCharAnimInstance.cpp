// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AnimInstance/BaseCharAnimInstance.h"
#include "Character/BaseChar.h"
#include "Components/SceneComponent.h"
#include "SimplePF.h"
#include "Util/DefineEnum.h"

void UBaseCharAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UBaseCharAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UBaseCharAnimInstance::Update_AnimState(EAnimState AnimState)
{
	//�Ǵ� �� ����� CharAnimStateComp�����ϰ� ���� BP �ִϸ��̼� �����

	FSM_State = AnimState;

	if (AActor* Owner = GetOwningActor())
	{
		UE_LOG(LogHJ, Log, TEXT("[%s] : UBaseCharAnimInstance : Update_AnimState = %s"), *Owner->GetName(), *UEnum::GetValueAsString(FSM_State));
	}

	//���� �ִϸ��̼� �ܿ� �߰� �ִϸ��̼� ó��(��Ÿ��)
	switch (AnimState)
	{
	case EAnimState::AnimState_Idle:	break;
	case EAnimState::AnimState_Move:	break;
	case EAnimState::AnimState_Shoot:
	{
		Montage_Play(Montage_Shoot);
	}
	break;
	case EAnimState::AnimState_Hit:
	{
		Montage_Stop(0.f, Montage_Shoot);
		Montage_Stop(0.f, Montage_Hit);
		Montage_Play(Montage_Hit);
	}
	break;
	case EAnimState::AnimState_Die:		break;

	default:
		break;
	}
}
