// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UserChar.h"
#include "Character/Components/CharHealthComp.h"
#include "Character/Components/CharWeaponComp.h"
#include "Manager/UIManagerSubsystem.h"
#include "TimerManager.h"
#include "Components/Button.h"

void UPlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UWorld* World = PC->GetWorld())
		{
			if (World->GetGameInstance())
			{
				UIManager = World->GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
			}
		}

		MyChar = Cast<AUserChar>(PC->GetPawn());
		if (MyChar.IsValid())
		{
			if (auto* HealthComp = MyChar->Get_HealthComp())
			{
				//내 캐릭터만 바인딩
				HealthComp->OnDel_MyCharHP.AddUObject(this, &UPlayWidget::UpdateHP);
				HealthComp->OnDel_MyCharDie.AddUObject(this, &UPlayWidget::OnDie);

				UpdateHP(HealthComp->Get_HP(), HealthComp->Get_MaxHP());
			}

			if (auto* WeaponComp = MyChar->Get_WeaponComp())
			{
				WeaponComp->OnDel_ChangeWeapon.AddUObject(this, &UPlayWidget::UpdateEquipWeapon);

				UpdateEquipWeapon(WeaponComp->GetCurWeaponType());
			}
		}
	}

	SetOverlay(EPlayWidgetState::EPlayWidgetState_Alive);
}

void UPlayWidget::HandleVisibilityChanged(ESlateVisibility InVisibility)
{
	Super::HandleVisibilityChanged(InVisibility);

	if (InVisibility == ESlateVisibility::Visible
		|| InVisibility == ESlateVisibility::SelfHitTestInvisible
		|| InVisibility == ESlateVisibility::HitTestInvisible)
	{
		// 켜졌을 때 처리
		Button_Quit->SetIsEnabled(true);
	}
	else
	{
		// 꺼졌을 때 처리 (Hidden/Collapsed 등)
	}
}

void UPlayWidget::QuitListenServer()
{
	if (auto* World = GEngine->GetCurrentPlayWorld())
	{
		if (World->GetGameInstance())
		{
			if (UUIManagerSubsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<UUIManagerSubsystem>())
			{
				UISubsystem->ShowWidget(EWidgetName::WBP_Loading);

				UISubsystem->DestroyWidget(EWidgetName::WBP_Play);
				UISubsystem->DestroyWidget(EWidgetName::WBP_HandGunScope);
				UISubsystem->DestroyWidget(EWidgetName::WBP_LifleScope);

				UGameplayStatics::OpenLevel(World, "SimplePFMap_Start");

				Button_Quit->SetIsEnabled(false);
			}
		}
	}

}
void UPlayWidget::SetOverlay(EPlayWidgetState WidgetState)
{
	if (!Overlay_Die || !Overlay_Hit)
		return;

	eWidgetState = WidgetState;

	Overlay_Hit->SetVisibility(ESlateVisibility::Collapsed);
	Overlay_Die->SetVisibility(ESlateVisibility::Collapsed);

	switch (WidgetState)
	{
	case EPlayWidgetState_Alive:
	{
	}
	break;
	case EPlayWidgetState_Hit:
	{
		Overlay_Hit->SetVisibility(ESlateVisibility::Visible);
	}
	break;
	case EPlayWidgetState_Die:
	{
		Overlay_Die->SetVisibility(ESlateVisibility::Visible);
	}
	break;

	default:
		break;
	}
}

void UPlayWidget::UpdateHP(float HP, float MaxHP)
{
	if (!ProgressBar_HP || !TextBlock_HP_Value)
		return;

	float fPercent = (float)HP / (float)MaxHP;
	ProgressBar_HP->SetPercent(fPercent);

	FString Str_HP_Value = FString::FromInt(HP) + TEXT(" / ") + FString::FromInt(MaxHP);
	TextBlock_HP_Value->SetText(FText::FromString(Str_HP_Value));

	if (GetWorld())
	{
		SetOverlay(EPlayWidgetState_Hit);

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (MyChar.IsValid())
				{
					if (auto* HealthComp = MyChar->Get_HealthComp())
					{
						if (HealthComp->Get_IsDie())
						{
							SetOverlay(EPlayWidgetState_Die);
						}
						else
						{
							SetOverlay(EPlayWidgetState_Alive);
						}
					}
				}
			}), 0.4f, false);
	}
}

void UPlayWidget::UpdateEquipWeapon(EWeaponType EquipWeapon)
{
	if (!UIManager.IsValid() || !HandGun_Equip || !Lifle_Equip)
		return;

	HandGun_Equip->SetVisibility(ESlateVisibility::Collapsed);
	Lifle_Equip->SetVisibility(ESlateVisibility::Collapsed);

	switch (EquipWeapon)
	{
	case EWeaponType::WeaponType_HandGun:
	{
		HandGun_Equip->SetVisibility(ESlateVisibility::Visible);

		//Lifle은 LeftCtl쪽에서 관리
		UIManager->ShowWidget(EWidgetName::WBP_HandGunScope);
	}
	break;
	case EWeaponType::WeaponType_Lifle:
	{
		Lifle_Equip->SetVisibility(ESlateVisibility::Visible);

		//Lifle은 LeftCtl쪽에서 관리
		UIManager->HideWidget(EWidgetName::WBP_HandGunScope);
	}
	break;

	default:
		break;
	}
}

void UPlayWidget::OnDie()
{
	SetOverlay(EPlayWidgetState_Die);
}