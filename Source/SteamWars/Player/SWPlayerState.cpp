#include "SWPlayerState.h"

#include "Characters/FPSCharacter/SWFPSCharacter.h"
#include "Components/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"
#include "Components/CharacterComponents/AbilitySystem/AttributeSet/SWAttributeSet.h"

ASWPlayerState::ASWPlayerState()
{
	NetUpdateFrequency = 100.f;

	AbilitySystemComponent = CreateDefaultSubobject<USWAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<USWAttributeSet>("AttributeSet");

	DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
}

UAbilitySystemComponent* ASWPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

USWAttributeSet* ASWPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

bool ASWPlayerState::IsAlive() const
{
	return GetHealth() > 0.0f;
}

void ASWPlayerState::ShowAbilityConfirmCancelText(bool ShowText)
{
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

int32 ASWPlayerState::GetCharacterLevel() const
{
	return AttributeSet->GetCharacterLevel();
}

void ASWPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate
		(AttributeSet->GetHealthAttribute()).AddUObject(this, &ASWPlayerState::HealthChanged);
		
		MaxHealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate
		(AttributeSet->GetMaxHealthAttribute()).AddUObject(this, &ASWPlayerState::MaxHealthChanged);
		
		HealthRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate
		(AttributeSet->GetHealthRegenRateAttribute()).AddUObject(this, &ASWPlayerState::HealthRegenRateChanged);
		
		StaminaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate
		(AttributeSet->GetStaminaAttribute()).AddUObject(this, &ASWPlayerState::StaminaChanged);
		
		MaxStaminaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate
		(AttributeSet->GetMaxStaminaAttribute()).AddUObject(this, &ASWPlayerState::MaxStaminaChanged);
		
		StaminaRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate
		(AttributeSet->GetStaminaRegenRateAttribute()).AddUObject(this, &ASWPlayerState::StaminaRegenRateChanged);
		
		CharacterLevelChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate
		(AttributeSet->GetCharacterLevelAttribute()).AddUObject(this, &ASWPlayerState::CharacterLevelChanged);

		
		AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(FName("State.Debuff.Stun")), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ASWPlayerState::StunTagChanged);
	}
}

void ASWPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
	float Health = Data.NewValue;

	// Update floating status bar
	ASWFPSCharacter* Hero = Cast<ASWFPSCharacter>(GetPawn());
	/*if (Hero)
	{
		UGDFloatingStatusBarWidget* HeroFloatingStatusBar = Hero->GetFloatingStatusBar();
		if (HeroFloatingStatusBar)
		{
			HeroFloatingStatusBar->SetHealthPercentage(Health / GetMaxHealth());
		}
	}*/

	// Update the HUD
	// Handled in the UI itself using the AsyncTaskAttributeChanged node as an example how to do it in Blueprint

	// If the player died, handle death
	if (!IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		if (Hero)
		{
			Hero->Die();
		}
	}
}

void ASWPlayerState::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	float MaxHealth = Data.NewValue;

	// Update floating status bar
	/*ASWFPSCharacter* Hero = Cast<ASWFPSCharacter>(GetPawn());
	if (Hero)
	{
		UGDFloatingStatusBarWidget* HeroFloatingStatusBar = Hero->GetFloatingStatusBar();
		if (HeroFloatingStatusBar)
		{
			HeroFloatingStatusBar->SetHealthPercentage(GetHealth() / MaxHealth);
		}
	}*/

	// Update the HUD
	/*APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC)
	{
		UGDHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetMaxHealth(MaxHealth);
		}
	}*/
}

void ASWPlayerState::HealthRegenRateChanged(const FOnAttributeChangeData& Data)
{
	float HealthRegenRate = Data.NewValue;

	// Update the HUD
	/*APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC)
	{
		UGDHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetHealthRegenRate(HealthRegenRate);
		}
	}*/
}

void ASWPlayerState::StaminaChanged(const FOnAttributeChangeData& Data)
{
	float Stamina = Data.NewValue;

	// Update the HUD
	// Handled in the UI itself using the AsyncTaskAttributeChanged node as an example how to do it in Blueprint
}

void ASWPlayerState::MaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	float MaxStamina = Data.NewValue;

	// Update the HUD
	/*APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC)
	{
		UGDHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetMaxStamina(MaxStamina);
		}
	}*/
}

void ASWPlayerState::StaminaRegenRateChanged(const FOnAttributeChangeData& Data)
{
	float StaminaRegenRate = Data.NewValue;

	// Update the HUD
	/*AGDPlayerController* PC = Cast<AGDPlayerController>(GetOwner());
	if (PC)
	{
		UGDHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetStaminaRegenRate(StaminaRegenRate);
		}
	}*/
}

void ASWPlayerState::CharacterLevelChanged(const FOnAttributeChangeData& Data)
{
	float CharacterLevel = Data.NewValue;

	// Update the HUD
	/*AGDPlayerController* PC = Cast<AGDPlayerController>(GetOwner());
	if (PC)
	{
		UGDHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetHeroLevel(CharacterLevel);
		}
	}*/
}

void ASWPlayerState::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
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
