#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "EnemySpawner.generated.h"

enum class ESpawnersID : uint8;
class AEnemyBaseCharacter;

UCLASS()
class STEAMWARS_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* SphereComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESpawnersID SpawnerID;
	
	UFUNCTION()
	void HandleEnemyDeath();
	
public:
	void SpawnEnemy();
	bool SpawnWeapon();
	
	void AddEnemyToQueue(const TSubclassOf<AEnemyBaseCharacter>& EnemyClass);
	void AddWeaponToQueue(const TSubclassOf<ARangeWeaponItem>& WeaponClass);

	ESpawnersID GetSpawnID() const;

	void DebugEnum(ESpawnersID SpawnType);

private:
	FTimerHandle TimerHandle;
	TArray<TSubclassOf<AEnemyBaseCharacter>> EnemyQueue;
	TSubclassOf<ARangeWeaponItem> WeaponQueue;
};
