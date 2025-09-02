// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Util/DefineEnum.h"
#include "Engine/StreamableManager.h"

#include "TableManagerSubsystem.generated.h"

class UDataTable;

DECLARE_MULTICAST_DELEGATE(FOnCompleteLoad)

UCLASS(Blueprintable)
class SIMPLEPF_API UTableManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UTableManagerSubsystem();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	//로딩 순서 종속 테이블 완료 됐는지
	FORCEINLINE bool Get_IsComplete_OrderLoad() const { return IsComplete_OrderLoad; }

	//완료 콜백 바인딩할 델리게이트
	FORCEINLINE FOnCompleteLoad& Get_DelCompleteOrderTB() { return Del_CompleteOrderTB; };

public:
	template<typename T>
	FORCEINLINE const T* Find_TableRow(ETableName TableType, FName SearchName) const
	{
		if (const UDataTable* Data = LoadDataTable.FindRef(TableType))
		{
			return Data->FindRow<T>(SearchName, FString::Printf(TEXT("Find_TableRow : Table = %d, Row = %s"), static_cast<int32>(TableType), *SearchName.ToString()));
		}

		return nullptr;
	}

private:
	void StartLoadTableList();
	void LoadCompleteLoadTableList();

	//순서 종속 로드
	void Load_OrderTable();
	void Per_CompleteLoad_OrderTable(ETableName TableName);

	//천천히 로드
	void Load_UnOrderTable();
	void Per_CompleteLoad_UnOrderTable(ETableName TableName);
	
private:
	//순서상 먼저 로드해야하는 테이블만 로드 완료했는지
	bool IsComplete_OrderLoad = false;
	FOnCompleteLoad Del_CompleteOrderTB;

	//모든 테이블 로드 완료
	bool IsComplete_AllLoad = false;

	TSharedPtr<FStreamableHandle> TB_LoadTable_Handle;

	//로드할 테이블 목록 테이블
	UPROPERTY(EditDefaultsOnly, Category = "TableManager", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UDataTable> TB_LoadTable;

	TQueue<TPair<ETableName, TSoftObjectPtr<UDataTable>>> Queue_OrderLoad;
	TQueue<TPair<ETableName, TSoftObjectPtr<UDataTable>>> Queue_UnOrderLoad;

	//사용중인 비동기 로딩 핸들
	TMap<ETableName, TSharedPtr<FStreamableHandle>> ActiveHandles;

	//<테이블 이름 <탐색 이름, 데이터>>
	UPROPERTY(Transient)
	TMap<ETableName,UDataTable*> LoadDataTable;
};
