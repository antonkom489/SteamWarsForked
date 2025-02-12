// Fill out your copyright notice in the Description page of Project Settings.


#include "SWSceneManager.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
USWSceneManager::USWSceneManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void USWSceneManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void USWSceneManager::RestartGame()
{
	// Получаем текущий уровень
	UWorld* World = GetWorld();
	if (World)
	{
		// Получаем имя текущего уровня
		FName CurrentLevelName = FName(*World->GetMapName());
        
		// Перезапускаем уровень
		UGameplayStatics::OpenLevel(World, CurrentLevelName);
	}
}

void USWSceneManager::TogglePause()
{
	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
	else
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}
