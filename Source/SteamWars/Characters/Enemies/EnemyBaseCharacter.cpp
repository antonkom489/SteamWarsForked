#include "EnemyBaseCharacter.h"

#include "Blueprint/UserWidget.h"
#include "Characters/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"
#include "Characters/CharacterComponents/AbilitySystem/AttributeSet/SWAttributeSet.h"
#include "Components/WidgetComponent.h"
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

	OnEnemyDied.Broadcast();
	Destroy();
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

void AEnemyBaseCharacter::Threated_Implementation(FVector ThreatedSource)
{
	
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

