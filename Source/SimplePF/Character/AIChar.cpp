// Fill out your copyright notice in the Description page of Project Settings.


#include "AIChar.h"

AAIChar::AAIChar()
{
	Tags.Add(TEXT("AIChar"));
}

void AAIChar::BeginPlay()
{
	Super::BeginPlay();
}

void AAIChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
