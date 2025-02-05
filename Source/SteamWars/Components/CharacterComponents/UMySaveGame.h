#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UMySaveGame.generated.h"

UCLASS()
class STEAMWARS_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FString PlayerName;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	int32 PlayerScore;
};

