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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawners")
	TSubclassOf<AEnemyBaseCharacter> EnemyChar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int WaveNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ESpawnersID, AEnemySpawner*> SpawnersMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float TransitionWaveTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int EnemyMaxCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int EnemyRemaining = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TMap<TSubclassOf<AEnemyBaseCharacter>, int> EnemyPool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	UDataTable* WaveDataTable;
	
	virtual void BeginPlay() override;
	void OnEnemyDefeated();
	
	FORCEINLINE FTimerHandle GetTimerHandle() const { return WaveTimer; }

private:
	FTimerHandle WaveTimer;

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
