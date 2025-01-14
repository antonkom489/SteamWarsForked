#include "USaveGameSW.h"
#include "Kismet/GameplayStatics.h"

USaveGameSW::USaveGameSW()
{
	SaveSlotName = TEXT("TestSaveSlot");
	ShootCount = 0;
}

void USaveGameSW::SaveGame(FString playerName, FString slotName, int32 ShootC)
{
		this->PlayerName = playerName;
	
		if (UGameplayStatics::SaveGameToSlot(this, slotName, ShootC))
		{
			UE_LOG(LogTemp, Display, TEXT("Saving save slot"));
		}
}

void USaveGameSW::LoadGame(FString slotName)
{
	if (UGameplayStatics::LoadGameFromSlot(slotName, 0))
	{
		// The operation was successful, so LoadedGame now contains the data we saved earlier.
		UE_LOG(LogTemp, Warning, TEXT("LOADED: %s"), *PlayerName);
	}
}
