// Fill out your copyright notice in the Description page of Project Settings.


#include "Unique/GameMode/PlayGameMode.h"
#include "EngineUtils.h"
#include "SimplePF.h"

#include "GameFramework/PlayerStart.h"

APlayGameMode::APlayGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/PF_Character/BP_UserChar.BP_UserChar_C"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

AActor* APlayGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	int32 RandomSpawn = FMath::RandRange(0, 3);
	FString StrRS = "PlayerStart_" + FString::FromInt(RandomSpawn);

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if(!PlayerStart)
			continue;

		if (PlayerStart->PlayerStartTag == FName(*StrRS))
		{
			UE_LOG(LogHJ, Log, TEXT("APlayGameMode : ChoosePlayerStart_Implementation : %s"), *StrRS);
			return PlayerStart;
		}
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}