#include "SWGameMode.h"
#include "EngineUtils.h"
#include "Characters/Enemies/EnemyBaseCharacter.h"
#include "Characters/Enemies/EnemySpawner.h"

void ASWGameMode::StartWave()
{
	UE_LOG(LogTemp, Warning, TEXT("Start Wave"));
	BuildEnemyPool();

	const int SpawnEnemyCount = FMath::Min(GetPoolSize(), EnemyMaxCount) - 1;
	
	for(int i = 0; i <= SpawnEnemyCount; i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("iter: %d"), i);
		SpawnEnemy();
	}
}

void ASWGameMode::EnterTransition()
{
	if(GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(WaveTimer, this, &ASWGameMode::NewWave, TransitionWaveTime, false);
	}
}

/*void ASWGameMode::BuildEnemyPool()
{
	if(WaveDataTable)
	{
		const FWaveDataFor* Pool = WaveDataTable->FindRow<FWaveDataFor>(
		FName(FString::FromInt(WaveNumber)), "");
		if(Pool)
		{
			for(const FWaveSpawnData& SpawnInfo : Pool->SpawnInfos)
			{
				// Проверяем, что EnemyClass валиден
				if(SpawnInfo.EnemyClass)
				{
					// Если класс уже есть в карте, увеличиваем количество
					if(EnemyPool.Contains(SpawnInfo.EnemyClass))
					{
						EnemyPool[SpawnInfo.EnemyClass] += SpawnInfo.EnemyCount;
					}
					else
					{
						// Если класса нет, добавляем его с начальным значением
						EnemyPool.Add(SpawnInfo.EnemyClass, SpawnInfo.EnemyCount);
					}
				}
			}
		}
	}
	
	EnemyRemaining = GetPoolSize();
	UE_LOG(LogTemp, Warning, TEXT("Enemy Remaining: %d"), EnemyRemaining);
}*/

void ASWGameMode::BuildEnemyPool()
{
	if(WaveDataTable)
	{
		const FWaveDataFor* Pool = WaveDataTable->FindRow<FWaveDataFor>(
		FName(FString::FromInt(WaveNumber)), "");
		if(Pool)
		{
			for(const FWaveSpawnData& SpawnInfo : Pool->SpawnInfos)
			{
				if (SpawnInfo.EnemyClass)
				{
					ESpawnersID SpawnerID = SpawnInfo.SpawnerID; // Предполагается, что у вас есть ID спавнера

					// Проверяем, есть ли такой спавнер в пуле
					if (!SpawnerEnemyPool.Contains(SpawnerID))
					{
						SpawnerEnemyPool.Add(SpawnerID, TMap<TSubclassOf<AEnemyBaseCharacter>, int32>());
					}

					// Добавляем врагов в соответствующий спавнер
					if (SpawnerEnemyPool[SpawnerID].Contains(SpawnInfo.EnemyClass))
					{
						SpawnerEnemyPool[SpawnerID][SpawnInfo.EnemyClass] += SpawnInfo.EnemyCount;
					}
					else
					{
						SpawnerEnemyPool[SpawnerID].Add(SpawnInfo.EnemyClass, SpawnInfo.EnemyCount);
					}
				}
			}
		}
	}
	
	EnemyRemaining = GetPoolSize();
	UE_LOG(LogTemp, Warning, TEXT("Enemy Remaining: %d"), EnemyRemaining);
}

/*int ASWGameMode::GetPoolSize() const
{
	TArray<int> CountEnemies;
	//EnemyPool.GenerateValueArray(CountEnemies);
	TMap<TSubclassOf<AEnemyBaseCharacter>, int> EnemiesPool;
	SpawnerEnemyPool.GenerateValueArray.GenerateValueArray(CountEnemies);
	//EnemiesPool.GenerateValueArray(CountEnemies);
	
	int ValueSum = 0;
	
	for(const int Value : CountEnemies)
	{
		ValueSum += Value;
	}

	return ValueSum;
}*/

int ASWGameMode::GetPoolSize() const
{
	int ValueSum = 0;

	for (const auto& SpawnerPair : SpawnerEnemyPool)
	{
		for (const auto& EnemyPair : SpawnerPair.Value)
		{
			ValueSum += EnemyPair.Value;
		}
	}

	return ValueSum;
}

void ASWGameMode::OnEnemyDefeated()
{
	
	EnemyRemaining -= 1;
	
	if(EnemyRemaining < EnemyMaxCount)
	{
		if(EnemyRemaining <= 0)
		{
			EnterTransition();
		}
	}
	else
	{
		SpawnEnemy();
	}
}

/*TSubclassOf<AEnemyBaseCharacter> ASWGameMode::GetEnemyFromPool()
{
	TArray<TSubclassOf<AEnemyBaseCharacter>> Keys;
	EnemyPool.GetKeys(Keys);
	if (Keys.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, Keys.Num() - 1);
		
		TSubclassOf<AEnemyBaseCharacter> LocalChar = Keys[RandomIndex];
		int* LocalCount = EnemyPool.Find(LocalChar);
		if(LocalCount && *LocalCount-1 > 0)
		{
			EnemyPool.Add(LocalChar, *LocalCount-1);
		}
		else
		{
			EnemyPool.Remove(LocalChar);
		}
		return LocalChar;
	}

	return EnemyChar;
}*/

TSubclassOf<AEnemyBaseCharacter> ASWGameMode::GetEnemyFromPool()
{
	if (SpawnerEnemyPool.Num() == 0)
	{
		return nullptr; // Если в пуле нет врагов, возвращаем nullptr
	}

	// 1. Выбираем случайный спавнер
	int32 RandomSpawnerIndex = FMath::RandRange(0, SpawnerEnemyPool.Num() - 1);
	auto SpawnerIterator = SpawnerEnemyPool.CreateIterator();

	for (int32 i = 0; i < RandomSpawnerIndex; ++i)
	{
		++SpawnerIterator;
	}

	ESpawnersID SelectedSpawnerID = SpawnerIterator->Key;
	TMap<TSubclassOf<AEnemyBaseCharacter>, int32>& EnemyMap = SpawnerIterator->Value;

	if (EnemyMap.Num() == 0)
	{
		SpawnerEnemyPool.Remove(SelectedSpawnerID);
		return GetEnemyFromPool(); // Рекурсивно вызываем снова, если спавнер оказался пуст
	}

	// 2. Выбираем случайного врага в выбранном спавнере
	int32 RandomEnemyIndex = FMath::RandRange(0, EnemyMap.Num() - 1);
	auto EnemyIterator = EnemyMap.CreateIterator();

	for (int32 i = 0; i < RandomEnemyIndex; ++i)
	{
		++EnemyIterator;
	}

	TSubclassOf<AEnemyBaseCharacter> SelectedEnemy = EnemyIterator->Key;
	int32& Count = EnemyIterator->Value;

	// 3. Обновляем количество врагов
	if (--Count <= 0)
	{
		EnemyIterator.RemoveCurrent(); // Удаляем запись, если врагов больше нет
	}

	// Если в спавнере больше нет врагов, удаляем его из пула
	if (EnemyMap.Num() == 0)
	{
		SpawnerEnemyPool.Remove(SelectedSpawnerID);
	}

	return SelectedEnemy;
}

void ASWGameMode::NewWave()
{
	WaveNumber += 1;
	UE_LOG(LogTemp, Warning, TEXT("New Wave"));
	StartWave();
}

void ASWGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeSpawners();
	NewWave();
}

/*void ASWGameMode::SpawnEnemy()
{
	// Получаем врага из пула
	TSubclassOf<AEnemyBaseCharacter> Enemy = GetEnemyFromPool();
	ESpawnersID TargetSpawnerID = GetSpawnerIDForEnemy(Enemy); // Метод для сопоставления

	if (TargetSpawnerID == ESpawnersID::None)
	{
		UE_LOG(LogTemp, Error, TEXT("No spawner found for enemy class!"));
		return;
	}

	// Ищем спавнер по ID
	if (AEnemySpawner** SpawnerPtr = SpawnersMap.Find(TargetSpawnerID))
	{
		AEnemySpawner* SpawnerInfo = *SpawnerPtr;
		if (SpawnerInfo)
		{
			SpawnerInfo->AddEnemyToQueue(Enemy);
			UE_LOG(LogTemp, Warning, TEXT("Added enemy to specific spawner queue."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawner with ID '%hhd' not found!"), TargetSpawnerID);
	}

	/*int RandomIndex = FMath::RandRange(0, SpawnersMap.Num() - 1);
	if(!GetSpawners().IsEmpty())
	{
		AEnemySpawner* SpawnerInfo = Cast<AEnemySpawner>(SpawnersMap[RandomIndex]);
		if(SpawnerInfo)
		{
			SpawnerInfo->AddEnemyToQueue(GetEnemyFromPool());
			UE_LOG(LogTemp, Warning, TEXT("Add enemy to Queue"));
		}
			
	}#1#
}*/

void ASWGameMode::SpawnEnemy()
{
	if (SpawnerEnemyPool.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnerEnemyPool is empty. No enemy spawned."));
		return;
	}

	// 1. Выбираем случайный спавнер с врагами
	int32 RandomSpawnerIndex = FMath::RandRange(0, SpawnerEnemyPool.Num() - 1);
	auto SpawnerIterator = SpawnerEnemyPool.CreateIterator();

	for (int32 i = 0; i < RandomSpawnerIndex; ++i)
	{
		++SpawnerIterator;
	}

	ESpawnersID SelectedSpawnerID = SpawnerIterator->Key;
	TMap<TSubclassOf<AEnemyBaseCharacter>, int32>& EnemyMap = SpawnerIterator->Value;

	if (EnemyMap.Num() == 0)
	{
		SpawnerEnemyPool.Remove(SelectedSpawnerID);
		return SpawnEnemy(); // Если в спавнере не осталось врагов, пробуем снова
	}

	// 2. Выбираем случайного врага из этого спавнера
	int32 RandomEnemyIndex = FMath::RandRange(0, EnemyMap.Num() - 1);
	auto EnemyIterator = EnemyMap.CreateIterator();

	for (int32 i = 0; i < RandomEnemyIndex; ++i)
	{
		++EnemyIterator;
	}

	TSubclassOf<AEnemyBaseCharacter> SelectedEnemy = EnemyIterator->Key;
	int32& Count = EnemyIterator->Value;

	// 3. Уменьшаем количество врагов или удаляем, если это был последний
	if (--Count <= 0)
	{
		EnemyIterator.RemoveCurrent();
	}

	// 4. Удаляем спавнер из пула, если там больше нет врагов
	if (EnemyMap.Num() == 0)
	{
		SpawnerEnemyPool.Remove(SelectedSpawnerID);
	}

	// 5. Получаем спавнер из `SpawnersMap` и добавляем врага в очередь
	AEnemySpawner* Spawner = SpawnersMap.FindRef(SelectedSpawnerID);
	if (!Spawner)
	{
		UE_LOG(LogTemp, Error, TEXT("Spawner with ID '%d' not found!"), static_cast<int32>(SelectedSpawnerID));
		return;
	}

	Spawner->AddEnemyToQueue(SelectedEnemy);
	UE_LOG(LogTemp, Warning, TEXT("Spawned enemy %s in spawner %d"), *SelectedEnemy->GetName(), static_cast<int32>(SelectedSpawnerID));
}

void ASWGameMode::InitializeSpawners()
{
	if(GetWorld())
	{
		for (TActorIterator<AEnemySpawner> It(GetWorld()); It; ++It)
		{
			AEnemySpawner* Spawner = *It;
			if (Spawner)
			{
				const ESpawnersID SpawnerID = Spawner->GetSpawnID();
				SpawnersMap.Add(SpawnerID, Spawner);
			}
		}
	}
}

ESpawnersID ASWGameMode::GetSpawnerIDForEnemy(TSubclassOf<AEnemyBaseCharacter> Enemy)
{
	const FWaveDataFor* Pool = WaveDataTable->FindRow<FWaveDataFor>(
		FName(FString::FromInt(WaveNumber)), "");
	if(Pool)
	{
		for (const FWaveSpawnData& SpawnInfo : Pool->SpawnInfos) // CurrentWaveData - текущие данные волны
		{
			if (SpawnInfo.EnemyClass == Enemy)
			{
				return SpawnInfo.SpawnerID;
			}
		}
	}
	
	return ESpawnersID::None; // Если спавнер не найден
}