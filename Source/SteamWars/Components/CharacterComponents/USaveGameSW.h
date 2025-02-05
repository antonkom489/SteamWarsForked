#pragma once

#include "UMySaveGame.h" // Подключите ваш класс сохранения
#include "GameFramework/SaveGame.h"

#include "USaveGameSW.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSaveGameComplete, const FString&, SlotName, int32, UserIndex, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLoadGameComplete, const FString&, SlotName, int32, UserIndex, USaveGame*, LoadedGameData);


UCLASS()
class STEAMWARS_API USaveGameSW : public USaveGame
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SaveGameData(const FString& SlotName, int32 PlayerScore);

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void LoadGameData(const FString& SlotName);

	UFUNCTION()
	void LoadGameDelegateFunction(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData);

	UFUNCTION()
	void SaveGameDelegateFunction(const FString& SlotName, const int32 UserIndex, bool bSuccess);
};
