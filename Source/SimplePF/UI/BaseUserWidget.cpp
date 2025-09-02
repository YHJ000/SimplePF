// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUserWidget.h"

void UBaseUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기화 1회

    OnNativeVisibilityChanged.AddUObject(this, &UBaseUserWidget::Bind_HandleVisibilityChanged);
}

void UBaseUserWidget::NativeDestruct()
{
    OnNativeVisibilityChanged.RemoveAll(this);

    Super::NativeDestruct();
}

void UBaseUserWidget::Bind_HandleVisibilityChanged(ESlateVisibility InVisibility)
{
    HandleVisibilityChanged(InVisibility);
}

void UBaseUserWidget::HandleVisibilityChanged(ESlateVisibility InVisibility)
{
    if (InVisibility == ESlateVisibility::Visible
        || InVisibility == ESlateVisibility::SelfHitTestInvisible
        || InVisibility == ESlateVisibility::HitTestInvisible)
    {
        // 켜졌을 때 처리
    }
    else
    {
        // 꺼졌을 때 처리 (Hidden/Collapsed 등)
    }
}

void UBaseUserWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	ViewportHandle.Reset();
}