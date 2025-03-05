// Fill out your copyright notice in the Description page of Project Settings.


#include "EndGame.h"
#include "Kismet/GameplayStatics.h"

void UEndGame::RestartGame()
{
	UWorld* World = GetWorld();
	if (World)
	{
		FName CurrentLevelName = FName(*World->GetMapName());
        
		UGameplayStatics::OpenLevel(World, CurrentLevelName);
	}
}