// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUserWidget.h"

void UBaseUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// �ʱ�ȭ 1ȸ

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
        // ������ �� ó��
    }
    else
    {
        // ������ �� ó�� (Hidden/Collapsed ��)
    }
}

void UBaseUserWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	ViewportHandle.Reset();
}