#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"
#include "AttributeSet.h"
#include "SWAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class STEAMWARS_API USWAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	USWAttributeSet();
	
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_Health, Category = "Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(USWAttributeSet, Health)

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_MaxHealth, Category = "MaxHealth")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(USWAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_HealthRegenRate)
	FGameplayAttributeData HealthRegenRate;
	ATTRIBUTE_ACCESSORS(USWAttributeSet, HealthRegenRate)

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_Stamina, Category = "Stamina")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(USWAttributeSet, Stamina)

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_Stamina, Category = "MaxStamina")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(USWAttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_StaminaRegenRate)
	FGameplayAttributeData StaminaRegenRate;
	ATTRIBUTE_ACCESSORS(USWAttributeSet, StaminaRegenRate)

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(USWAttributeSet, Damage)

	UPROPERTY(BlueprintReadOnly, Category = "MoveSpeed", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(USWAttributeSet, MoveSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "CharacterLevel", ReplicatedUsing = OnRep_CharacterLevel)
	FGameplayAttributeData CharacterLevel;
	ATTRIBUTE_ACCESSORS(USWAttributeSet, CharacterLevel)

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

protected:
	FGameplayTag HeadShotTag;
	FGameplayTag HitDirectionFrontTag;
	FGameplayTag HitDirectionBackTag;
	FGameplayTag HitDirectionRightTag;
	FGameplayTag HitDirectionLeftTag;
	
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate) const;

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;

	UFUNCTION()
	virtual void OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate);

	UFUNCTION()
	virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

	UFUNCTION()
	virtual void OnRep_CharacterLevel(const FGameplayAttributeData& OldCharacterLevel);
};
