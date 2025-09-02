// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StartWidget.h"
#include "Kismet/GamePlayStatics.h"
#include "Manager/UIManagerSubsystem.h"
#include "Components/Button.h"

void UStartWidget::HandleVisibilityChanged(ESlateVisibility InVisibility)
{
	Super::HandleVisibilityChanged(InVisibility);

	if (InVisibility == ESlateVisibility::Visible
		|| InVisibility == ESlateVisibility::SelfHitTestInvisible
		|| InVisibility == ESlateVisibility::HitTestInvisible)
	{
		// 켜졌을 때 처리
		SetButtonsEnable(true);
	}
	else
	{
		// 꺼졌을 때 처리 (Hidden/Collapsed 등)
	}
}

void UStartWidget::HostListenServer()
{
	if (auto* World = GEngine->GetCurrentPlayWorld())
	{
		if (World->GetGameInstance())
		{
			if (UUIManagerSubsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<UUIManagerSubsystem>())
			{
				UISubsystem->ShowWidget(EWidgetName::WBP_Loading);

				UGameplayStatics::OpenLevel(World, "SimplePFMap_Play", true, "?listen");

				SetButtonsEnable(false);
			}
		}
	}
}

void UStartWidget::JoinListenServer()
{
	if (auto* World = GEngine->GetCurrentPlayWorld())
	{
		if (World->GetGameInstance())
		{
			if (UUIManagerSubsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<UUIManagerSubsystem>())
			{
				UISubsystem->ShowWidget(EWidgetName::WBP_Loading);

				UGameplayStatics::OpenLevel(World, "127.0.0.1", true, "?join");

				SetButtonsEnable(false);
			}
		}
	}
}

void UStartWidget::SetButtonsEnable(bool bEnable)
{
	if (!Button_Host || !Button_Join)
		return;

	Button_Host->SetIsEnabled(false);
	Button_Join->SetIsEnabled(false);
}