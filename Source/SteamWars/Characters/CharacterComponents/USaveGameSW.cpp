#include "USaveGameSW.h"
#include "Kismet/GameplayStatics.h"

void USaveGameSW::SaveGameData(const FString& SlotName, int32 PlayerScore)
{
	if (UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass())))
	{
		SaveGameInstance->PlayerName = TEXT("PlayerOne");
		SaveGameInstance->PlayerScore = PlayerScore; // Установите значение int32

		FAsyncSaveGameToSlotDelegate SavedDelegate;
		SavedDelegate.BindUObject(this, &USaveGameSW::SaveGameDelegateFunction);

		UGameplayStatics::AsyncSaveGameToSlot(SaveGameInstance, SlotName, 0, SavedDelegate);
	}
}

void USaveGameSW::LoadGameData(const FString& SlotName)
{
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &USaveGameSW::LoadGameDelegateFunction);

	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, 0, LoadedDelegate);
}

void USaveGameSW::LoadGameDelegateFunction(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	if (UMySaveGame* LoadedSaveGame = Cast<UMySaveGame>(LoadedGameData))
	{
		FString PlayerName = LoadedSaveGame->PlayerName;
		int32 PlayerScore = LoadedSaveGame->PlayerScore;

		UE_LOG(LogTemp, Log, TEXT("Player Name: %s, Player Score: %d"), *PlayerName, PlayerScore);
	}
}

void USaveGameSW::SaveGameDelegateFunction(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Game saved successfully in slot: %s"), *SlotName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save game in slot: %s"), *SlotName);
	}
}