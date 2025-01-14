#include "EnemySpawner.h"
#include "EnemyBaseCharacter.h"
#include "Components/SphereComponent.h"
#include "GameMode/SWGameMode.h"


AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComp->SetupAttachment(GetRootComponent());
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	DebugEnum(GetSpawnID());
	
	if(GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AEnemySpawner::SpawnEnemy, 2.0f, true);
	}
}

void AEnemySpawner::SpawnEnemy()
{
	if(EnemyQueue.IsEmpty()) return;

	TArray<TSubclassOf<AEnemyBaseCharacter>> EnemiesToRemove;
	
	for(TSubclassOf<AEnemyBaseCharacter> Enemy : EnemyQueue)
	{
		if(Enemy && GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AEnemyBaseCharacter* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBaseCharacter>(Enemy, GetActorTransform(), SpawnParams);
			if(SpawnedEnemy)
			{
				SpawnedEnemy->OnEnemyDied.AddDynamic(this, &AEnemySpawner::HandleEnemyDeath);
			}
			
			EnemiesToRemove.Add(Enemy);
		}
	}

	EnemyQueue.RemoveAll([&](const TSubclassOf<AEnemyBaseCharacter>& Enemy)
   {
	   return EnemiesToRemove.Contains(Enemy);
   });
}

void AEnemySpawner::AddEnemyToQueue(const TSubclassOf<AEnemyBaseCharacter>& EnemyClass)
{
	if(EnemyClass)
	{
		EnemyQueue.Add(EnemyClass);
	
		TArray<AActor*> OverlappingActors;
		SphereComp->GetOverlappingActors(OverlappingActors, EnemyClass);

		if(OverlappingActors.IsEmpty())
		{
			SpawnEnemy();
		}
	}
}

void AEnemySpawner::HandleEnemyDeath()
{
	if(GetWorld())
	{
		ASWGameMode* GameMode = Cast<ASWGameMode>(GetWorld()->GetAuthGameMode());
		GameMode->OnEnemyDefeated();
	}
}

ESpawnersID AEnemySpawner::GetSpawnID() const
{
	return SpawnerID;
}

void AEnemySpawner::DebugEnum(ESpawnersID SpawnType)
{
	FString EnumString = UEnum::GetValueAsString(SpawnType);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, EnumString);
	}
}

