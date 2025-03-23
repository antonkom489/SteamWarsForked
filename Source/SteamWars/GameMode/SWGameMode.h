#pragma once

#include "CoreMinimal.h"
#include "SWTypes.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
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
struct FWaveWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ARangeWeaponItem> WeaponItemClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESpawnersID SpawnerID = ESpawnersID::Weapon;
};

USTRUCT(BlueprintType)
struct FWaveDataFor : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWaveSpawnData> SpawnInfos;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWaveWeaponData> Weapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int EnemyMaxCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionWaveTime = 3.0f;
};

UCLASS()
class STEAMWARS_API ASWGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	

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

	float TransitionWaveTime = 3.0f;
	int EnemyMaxCount = 5;
	FTimerHandle WaveTimer;
	TSubclassOf<AEnemyBaseCharacter> EnemyChar;
	TMap<ESpawnersID, AEnemySpawner*> SpawnersMap;
	int WaveNumber = 0;
	int EnemyRemaining = 0;
	TMap<TSubclassOf<AEnemyBaseCharacter>, int> EnemyPool;
	TMap<ESpawnersID, TMap<TSubclassOf<AEnemyBaseCharacter>, int32>> SpawnerEnemyPool;

	void SpawnEnemy();
	void StartWave();
	void EnterTransition();
	void BuildEnemyPool();
	int GetPoolSize() const;
	void InitializeSpawners();
	void SpawnWeapon();
	ESpawnersID GetSpawnerIDForEnemy(TSubclassOf<AEnemyBaseCharacter> Enemy);

	static TArray<TSubclassOf<ARangeWeaponItem>> GetWeaponClasses(const FWaveDataFor& WaveData);
	TSubclassOf<ARangeWeaponItem> GetWeaponClassFromDataTable(UDataTable* WaveDataTable, int32 WaveIndex);
	
	TSubclassOf<AEnemyBaseCharacter> GetEnemyFromPool();
	void NewWave();
};
