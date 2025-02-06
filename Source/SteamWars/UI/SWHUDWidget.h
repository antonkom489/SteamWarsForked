#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SWHUDWidget.generated.h"

class UPaperSprite;
class UTexture2D;

UCLASS()
class STEAMWARS_API USWHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowAbilityConfirmPrompt(bool bShowText);
	
	/**
	* Weapon info
	*/

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEquippedWeaponSprite(class UPaperSprite* Sprite);

	// Things like fire mode for rifle
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEquippedWeaponStatusText(const FText& StatusText);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPrimaryClipAmmo(int32 ClipAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPrimaryReserveAmmo(int32 ReserveAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetSecondaryClipAmmo(int32 SecondaryClipAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetSecondaryReserveAmmo(int32 SecondaryReserveAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetReticle(TSubclassOf<class USWHUDReticle> ReticleClass);


	/**
	* Attribute setters
	*/

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxHealth(float MaxHealth);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentHealth(float CurrentHealth);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHealthPercentage(float HealthPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHealthRegenRate(float HealthRegenRate);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxStamina(float MaxStamina);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentStamina(float CurrentStamina);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetStaminaPercentage(float StaminaPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetStaminaRegenRate(float StaminaRegenRate);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHeroLevel(int32 HeroLevel);
};
