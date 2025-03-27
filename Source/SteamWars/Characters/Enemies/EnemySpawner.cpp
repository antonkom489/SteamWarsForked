#include "EnemySpawner.h"
#include "EnemyBaseCharacter.h"
#include "Components/SphereComponent.h"
#include "GameMode/SWGameMode.h"
#include "Kismet/KismetSystemLibrary.h"


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

			UKismetSystemLibrary::Delay(this, 1.0f, FLatentActionInfo());
			
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

bool AEnemySpawner::SpawnWeapon()
{
	if (WeaponQueue)
	{
		GetWorld()->SpawnActor<ARangeWeaponItem>(WeaponQueue, GetActorTransform());

		WeaponQueue = nullptr;
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid index or weapon class is null"));
		return false;
	}
}

void AEnemySpawner::AddWeaponToQueue(const TSubclassOf<ARangeWeaponItem>& WeaponClass)
{
	if (WeaponClass) 
	{
		WeaponQueue = WeaponClass; 
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon classes to add to the queue."));
	}
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

