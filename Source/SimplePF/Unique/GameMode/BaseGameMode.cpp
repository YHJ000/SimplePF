// Fill out your copyright notice in the Description page of Project Settings.


#include "Unique/GameMode/BaseGameMode.h"

ABaseGameMode::ABaseGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/PF_Character/BP_UserChar.BP_UserChar_C"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}