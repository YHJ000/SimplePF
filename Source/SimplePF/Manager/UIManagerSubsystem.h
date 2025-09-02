// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DefineEnum.h"
#include "Engine/StreamableManager.h"

#include "UIManagerSubsystem.generated.h"

class UBaseUserWidget;
class UWorld;
class UUserWidget;
class UTableManagerSubsystem;

UCLASS(Blueprintable)
class SIMPLEPF_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UUIManagerSubsystem();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//���� ��ȯ�� ȣ��
	void OnPreLoad(const FString& MapName);
	void OnPostLoad(UWorld* LoadedWorld);
	void OnPostWorldInit(UWorld* LoadedWorld, const UWorld::InitializationValues IVS);

public:
	//Get
	TWeakObjectPtr<UBaseUserWidget> GetWidgetWeakPtr(EWidgetName WidgetName) const;

	//ǥ��/����/����
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void ShowWidget(EWidgetName WidgetName, int32 ZOrder = 5);

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void HideWidget(EWidgetName WidgetName);

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void DestroyWidget(EWidgetName WidgetName);

private:
	//Loading
	void Process_CreateLoadingPopup();
	void Process_RemoveLoadingPopup(UWorld* LoadedWorld);
	void Direct_CreateLoadingPopup();
	void Direct_RemoveLoadingPopup();

	void LoadCompleteCreateWidget(EWidgetName WidgetName, int32 ZOrder);
	void RemoveViewportWidget(UBaseUserWidget* WidgetPtr);

private:
	UPROPERTY(Transient)
	bool IsEnable_WidgetTable = false;

	UPROPERTY(Transient)
	bool IsEnable_FirstMapLoaded = false;

	//�ε� �˾���
	UPROPERTY(Transient)
	TSubclassOf<UBaseUserWidget>	LoadingUIClass;		//��� �ε��

	UPROPERTY(Transient)
	TWeakObjectPtr<UBaseUserWidget> LoadingWidget;		//�����ϸ� AddViewport ����

	//Show - Hide ����
	UPROPERTY(Transient)
	TMap<EWidgetName, TObjectPtr<UBaseUserWidget>> Recycle_Widget;

	//Show - Destory ���� ���
	UPROPERTY(Transient)
	TMap<EWidgetName, TObjectPtr<UBaseUserWidget>> Using_Widget;

	//������� �񵿱� �ε� �ڵ�
	TMap<EWidgetName, TSharedPtr<FStreamableHandle>> ActiveHandles;
};
