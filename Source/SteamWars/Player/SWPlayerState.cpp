#include "SWPlayerState.h"

#include "SWPlayerController.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"
#include "Characters/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"
#include "Characters/CharacterComponents/AbilitySystem/AttributeSet/AmmoAttributeSet.h"
#include "Characters/CharacterComponents/AbilitySystem/AttributeSet/SWAttributeSet.h"
#include "UI/SWHUDWidget.h"

ASWPlayerState::ASWPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<USWAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another GDPlayerState (Hero) receives a GE,
	// we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create the attribute set, this replicates by default
	// Adding it as a subobject of the owning actor of an AbilitySystemComponent
	// automatically registers the AttributeSet with the AbilitySystemComponent
	AttributeSet = CreateDefaultSubobject<USWAttributeSet>(TEXT("AttributeSetBase"));

	AmmoAttributeSet = CreateDefaultSubobject<UAmmoAttributeSet>(TEXT("AmmoAttributeSet"));

	// Set PlayerState's NetUpdateFrequency to the same as the Character.
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	// 100 is probably way too high for a shipping game, you can adjust to fit your needs.
	NetUpdateFrequency = 100.0f;

	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
}

UAbilitySystemComponent* ASWPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

USWAttributeSet* ASWPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

class UAmmoAttributeSet* ASWPlayerState::GetAmmoAttributeSet() const
{
	return AmmoAttributeSet;
}

bool ASWPlayerState::IsAlive() const
{
	return GetHealth() > 0.0f;
}

void ASWPlayerState::ShowAbilityConfirmCancelText(bool ShowText)
{
	ASWPlayerController* PC = Cast<ASWPlayerController>(GetOwner());
	if (PC)
	{
		USWHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->ShowAbilityConfirmPrompt(ShowText);
		}
	}
}

float ASWPlayerState::GetHealth() const
{
	return AttributeSet->GetHealth();
}

float ASWPlayerState::GetMaxHealth() const
{
	return AttributeSet->GetMaxHealth();
}

float ASWPlayerState::GetHealthRegenRate() const
{
	return AttributeSet->GetHealthRegenRate();
}

float ASWPlayerState::GetStamina() const
{
	return AttributeSet->GetStamina();
}

float ASWPlayerState::GetMaxStamina() const
{
	return AttributeSet->GetMaxStamina();
}

float ASWPlayerState::GetStaminaRegenRate() const
{
	return AttributeSet->GetStaminaRegenRate();
}

float ASWPlayerState::GetMoveSpeed() const
{
	return AttributeSet->GetMoveSpeed();
}

int32 ASWPlayerState::GetCharacterLevel() const
{
	return AttributeSet->GetCharacterLevel();
}

int32 ASWPlayerState::GetPrimaryClipAmmo() const
{
	ASWFPSCharacter* Hero = GetPawn<ASWFPSCharacter>();
	if (Hero)
	{
		return Hero->GetPrimaryClipAmmo();
	}

	return 0;
}

int32 ASWPlayerState::GetPrimaryReserveAmmo() const
{
	ASWFPSCharacter* Hero = GetPawn<ASWFPSCharacter>();
	if (Hero && Hero->GetCurrentWeapon() && AmmoAttributeSet)
	{
		FGameplayAttribute Attribute = AmmoAttributeSet->GetReserveAmmoAttributeFromTag(Hero->GetCurrentWeapon()->PrimaryAmmoType);
		if (Attribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(Attribute);
		}
	}

	return 0;
}

void ASWPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate
		(AttributeSet->GetHealthAttribute()).AddUObject(this, &ASWPlayerState::HealthChanged);
	}
}

void ASWPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
	float Health = Data.NewValue;

	// Update floating status bar
	ASWFPSCharacter* Hero = Cast<ASWFPSCharacter>(GetPawn());
	
	if (IsValid(Hero) && !IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		if (Hero)
		{
			Hero->Die();
		}
	}
}
