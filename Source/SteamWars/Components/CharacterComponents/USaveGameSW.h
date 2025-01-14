#pragma once

#include "GameFramework/SaveGame.h"
#include "USaveGameSW.generated.h"

UCLASS()
class STEAMWARS_API USaveGameSW : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString PlayerName;
 
	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName;
 
	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 ShootCount;
 
	USaveGameSW();

	private:
	void SaveGame(FString playerName, FString slotName, int32 ShootC);
	void LoadGame(FString slotName);
};
