#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerState.h"
#include "SWPlayerState.generated.h"

struct FOnAttributeChangeData;
class USWAttributeSet;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGSOnGameplayAttributeValueChangedDelegate, FGameplayAttribute,
                                               Attribute, float, NewValue, float, OldValue);


UCLASS()
class STEAMWARS_API ASWPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	ASWPlayerState();
	
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	virtual USWAttributeSet* GetAttributeSet() const;
	class UAmmoAttributeSet* GetAmmoAttributeSet() const;

	UFUNCTION(BlueprintCallable, Category = "SWPlayerState")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|UI")
	void ShowAbilityConfirmCancelText(bool ShowText);

	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|Attributes")
	float GetMaxHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|Attributes")
	float GetHealthRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|Attributes")
	float GetStaminaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|Attributes")
	float GetMoveSpeed() const;
	
	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|Attributes")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|Attributes")
	int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "SWPlayerState|Attributes")
	int32 GetPrimaryReserveAmmo() const;
	
protected:
	FGameplayTag DeadTag;
	
	UPROPERTY()
	class USWAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	class USWAttributeSet* AttributeSet;

	UPROPERTY()
	class UAmmoAttributeSet* AmmoAttributeSet;

	// Attribute changed delegate handles
	FDelegateHandle HealthChangedDelegateHandle;

	virtual void BeginPlay() override;

	// Attribute changed callbacks
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
};
