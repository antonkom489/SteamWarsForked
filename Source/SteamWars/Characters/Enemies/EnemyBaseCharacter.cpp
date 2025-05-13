#include "EnemyBaseCharacter.h"

#include "Blueprint/UserWidget.h"
#include "Characters/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"
#include "Characters/CharacterComponents/AbilitySystem/AttributeSet/SWAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/SWFloatingStatusBarWidget.h"

AEnemyBaseCharacter::AEnemyBaseCharacter(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	HardRefAbilitySystemComponent = CreateDefaultSubobject<USWAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	HardRefAbilitySystemComponent->SetIsReplicated(true);

	// Minimal Mode means that no GameplayEffects will replicate. They will only live on the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	HardRefAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// Set our parent's TWeakObjectPtr
	AbilitySystemComponent = HardRefAbilitySystemComponent;

	// Create the attribute set, this replicates by default
	// Adding it as a subobject of the owning actor of an AbilitySystemComponent
	// automatically registers the AttributeSet with the AbilitySystemComponent
	HardRefAttributeSet = CreateDefaultSubobject<USWAttributeSet>(TEXT("AttributeSetBase"));

	// Set our parent's TWeakObjectPtr
	AttributeSet = HardRefAttributeSet;
}

void AEnemyBaseCharacter::FinishDying()
{
	
	Super::FinishDying();
	
	DiedEnemy(); 
}

void AEnemyBaseCharacter::DiedEnemy_Implementation()
{
	
	OnEnemyDied.Broadcast();

	// Активируем ragdoll
	EnableRagdoll();

	
	// Запускаем таймер на уничтожение
	GetWorld()->GetTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&AEnemyBaseCharacter::DestroyEnemyActor,
		TimeToDestroy,
		false
	);
	
}

void AEnemyBaseCharacter::DestroyEnemyActor()
{
	Destroy();
}

void AEnemyBaseCharacter::EnableRagdoll()
{
	// Отключаем движение
	GetCharacterMovement()->DisableMovement();

	// Отключаем столкновение капсулы
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Устанавливаем профиль коллизии для ragdoll
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	// Отключаем текущую анимацию
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationCustomMode);

	// Включаем симуляцию физики
	GetMesh()->SetSimulatePhysics(true);

	// Обнуляем velocity на случай, если персонаж двигался
	GetMesh()->SetAllPhysicsLinearVelocity(FVector::ZeroVector);

	FVector LaunchImpulse = GetActorForwardVector() * BackImpulse + Impulse; // Назад + вверх
	GetMesh()->AddImpulseToAllBodiesBelow(LaunchImpulse, TEXT("Bone"), true);
}


void AEnemyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitDefaultAttributes();
		AddStartupEffects();
		GiveDefaultAbilities();

		InitializeFloatingStatusBar();

		// Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate
		(AttributeSet->GetHealthAttribute()).AddUObject(this, &AEnemyBaseCharacter::HealthChanged);

		// Tag change callbacks
		AbilitySystemComponent->RegisterGameplayTagEvent
		(FGameplayTag::RequestGameplayTag(FName("State.Debuff.Stun")),
			EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AEnemyBaseCharacter::StunTagChanged);
	}

	LastCheckedLocation = GetActorLocation();

	GetWorld()->GetTimerManager().SetTimer(
		MovementCheckTimer,
		this,
		&AEnemyBaseCharacter::CheckStandingStill,
		MovementCheckInterval,
		true
	);
}

void AEnemyBaseCharacter::HealthChanged(const FOnAttributeChangeData& Data)
{
	float Health = Data.NewValue;

	// Update floating status bar
	if (UIFloatingStatusBar)
	{
		UIFloatingStatusBar->SetHealthPercentage(Health / GetMaxHealth());
	}

	// If the minion died, handle death
	if (!IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		Die();
	}
}

void AEnemyBaseCharacter::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		FGameplayTagContainer AbilityTagsToCancel;
		AbilityTagsToCancel.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));

		FGameplayTagContainer AbilityTagsToIgnore;
		AbilityTagsToIgnore.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.NotCanceledByStun")));

		AbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel, &AbilityTagsToIgnore);
	}
}

void AEnemyBaseCharacter::CheckStandingStill()
{
	FVector CurrentLocation = GetActorLocation();
	float DistanceMoved = FVector::DistSquared(CurrentLocation, LastCheckedLocation);

	const float Tolerance = 5.0f; // квадрат расстояния в см^2

	if (DistanceMoved < Tolerance)
	{
		TimeNotMoving += MovementCheckInterval;

		if (TimeNotMoving >= RequiredStillTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemy has been standing still for %.1f seconds"), TimeNotMoving);
			FVector NewLocation = FVector(1500.f, 600.f, 200.f);
			FRotator NewRotation = FRotator::ZeroRotator;

			TeleportTo(NewLocation, NewRotation, false, true);
		}
	}
	else
	{
		TimeNotMoving = 0.0f; // сброс таймера, враг двигается
	}

	LastCheckedLocation = CurrentLocation;
}

void AEnemyBaseCharacter::Threated_Implementation(FVector ThreatedSource)
{
	FVector CurrentLocation = GetActorLocation();
	float DistanceMoved = FVector::DistSquared(CurrentLocation, LastCheckedLocation);

	const float Tolerance = 5.0f; // квадрат расстояния в см^2

	if (DistanceMoved < Tolerance)
	{
		TimeNotMoving += MovementCheckInterval;

		if (TimeNotMoving >= RequiredStillTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemy has been standing still for %.1f seconds"), TimeNotMoving);
			// здесь можешь вызвать нужную функцию или флаг
		}
	}
	else
	{
		TimeNotMoving = 0.0f; // сброс таймера, враг двигается
	}

	LastCheckedLocation = CurrentLocation;
}

void AEnemyBaseCharacter::EnterCover_Implementation()
{
	bIsInCover = true;
	Crouch();
}

void AEnemyBaseCharacter::ShootAI()
{
	AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(ESWAbilityInputID::ShootAI));
}

void AEnemyBaseCharacter::ShotAI()
{
	AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(ESWAbilityInputID::ShotAI));
}

