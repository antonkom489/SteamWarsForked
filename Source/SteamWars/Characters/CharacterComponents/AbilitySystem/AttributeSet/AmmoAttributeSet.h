#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "AmmoAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class STEAMWARS_API UAmmoAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAmmoAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_NormalReserveAmmo)
	FGameplayAttributeData NormalReserveAmmo;
	ATTRIBUTE_ACCESSORS(UAmmoAttributeSet, NormalReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxNormalReserveAmmo)
	FGameplayAttributeData MaxNormalReserveAmmo;
	ATTRIBUTE_ACCESSORS(UAmmoAttributeSet, MaxNormalReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_HeavyReserveAmmo)
	FGameplayAttributeData HeavyReserveAmmo;
	ATTRIBUTE_ACCESSORS(UAmmoAttributeSet, HeavyReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxHeavyReserveAmmo)
	FGameplayAttributeData MaxHeavyReserveAmmo;
	ATTRIBUTE_ACCESSORS(UAmmoAttributeSet, MaxHeavyReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_SpecialReserveAmmo)
	FGameplayAttributeData SpecialReserveAmmo;
	ATTRIBUTE_ACCESSORS(UAmmoAttributeSet, SpecialReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxSpecialReserveAmmo)
	FGameplayAttributeData MaxSpecialReserveAmmo;
	ATTRIBUTE_ACCESSORS(UAmmoAttributeSet, MaxSpecialReserveAmmo)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	static FGameplayAttribute GetReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag);
	static FGameplayAttribute GetMaxReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag);

protected:
	// Cache tags
	FGameplayTag NormalAmmoTag;
	FGameplayTag HeavyAmmoTag;
	FGameplayTag SpecialAmmoTag;

	// Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes.
	// (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before)
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

	/**
	* These OnRep functions exist to make sure that the ability system internal representations are synchronized properly during replication
	**/
	
	UFUNCTION()
	virtual void OnRep_NormalReserveAmmo(const FGameplayAttributeData& OldNormalReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxNormalReserveAmmo(const FGameplayAttributeData& OldMaxNormalReserveAmmo);

	UFUNCTION()
	virtual void OnRep_HeavyReserveAmmo(const FGameplayAttributeData& OldHeavyReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxHeavyReserveAmmo(const FGameplayAttributeData& OldMaxHeavyReserveAmmo);

	UFUNCTION()
	virtual void OnRep_SpecialReserveAmmo(const FGameplayAttributeData& OldSpecialReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxSpecialReserveAmmo(const FGameplayAttributeData& OldMaxSpecialReserveAmmo);
};
