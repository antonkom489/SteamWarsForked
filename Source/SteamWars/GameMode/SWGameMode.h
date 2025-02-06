#pragma once

#include "CoreMinimal.h"
#include "SWTypes.h"
#include "GameFramework/GameMode.h"
#include "SWGameMode.generated.h"


class AEnemyBaseCharacter;
class AEnemySpawner;


USTRUCT(BlueprintType)
struct FWaveSpawnData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyBaseCharacter> EnemyClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int EnemyCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESpawnersID SpawnerID = ESpawnersID::None;
};

USTRUCT(BlueprintType)
struct FWaveDataFor : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWaveSpawnData> SpawnInfos;
};

UCLASS()
class STEAMWARS_API ASWGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float TransitionWaveTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int EnemyMaxCount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	UDataTable* WaveDataTable;
	
	virtual void BeginPlay() override;
	void OnEnemyDefeated();
	
	FORCEINLINE FTimerHandle GetTimerHandle() const { return WaveTimer; }

	UFUNCTION(BlueprintCallable, Category = "Wave")
	int32 GetWaveNumber() const { return WaveNumber; }

	UFUNCTION(BlueprintCallable, Category = "Wave")
	int32 GetEnemyRemaining() const { return EnemyRemaining; };

private:
	FTimerHandle WaveTimer;

	TSubclassOf<AEnemyBaseCharacter> EnemyChar;
	TMap<ESpawnersID, AEnemySpawner*> SpawnersMap;
	int WaveNumber = 0;
	int EnemyRemaining = 0;
	TMap<TSubclassOf<AEnemyBaseCharacter>, int> EnemyPool;
	TMap<int32, TMap<TSubclassOf<AEnemyBaseCharacter>, int32>> SpawnerEnemyPool;
	
	void SpawnEnemy();
	void StartWave();
	void EnterTransition();
	void BuildEnemyPool();
	int GetPoolSize() const;
	void InitializeSpawners();
	ESpawnersID GetSpawnerIDForEnemy(TSubclassOf<AEnemyBaseCharacter> Enemy);
	TSubclassOf<AEnemyBaseCharacter> GetEnemyFromPool();
	void NewWave();
};
