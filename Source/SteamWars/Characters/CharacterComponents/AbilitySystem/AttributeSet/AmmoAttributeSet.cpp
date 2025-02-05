#include "AmmoAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"
#include "Net/UnrealNetwork.h"

UAmmoAttributeSet::UAmmoAttributeSet()
{
	NormalAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Normal"));
	HeavyAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Heavy"));
	SpecialAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Special"));
}

void UAmmoAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UAmmoAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetNormalReserveAmmoAttribute())
	{
		float Ammo = GetNormalReserveAmmo();
		SetNormalReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxNormalReserveAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetHeavyReserveAmmoAttribute())
	{
		float Ammo = GetHeavyReserveAmmo();
		SetHeavyReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxHeavyReserveAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetSpecialReserveAmmoAttribute())
	{
		float Ammo = GetSpecialReserveAmmo();
		SetSpecialReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxSpecialReserveAmmo()));
	}
}

void UAmmoAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, NormalReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, MaxNormalReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, HeavyReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, MaxHeavyReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, SpecialReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, MaxSpecialReserveAmmo, COND_None, REPNOTIFY_Always);
}

FGameplayAttribute UAmmoAttributeSet::GetReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag)
{
	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Normal")))
	{
		return GetNormalReserveAmmoAttribute();
	}
	else if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Heavy")))
	{
		return GetHeavyReserveAmmoAttribute();
	}
	else if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Special")))
	{
		return GetSpecialReserveAmmoAttribute();
	}

	return FGameplayAttribute();
}

FGameplayAttribute UAmmoAttributeSet::GetMaxReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag)
{
	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Normal")))
	{
		return GetMaxNormalReserveAmmoAttribute();
	}
	else if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Heavy")))
	{
		return GetMaxHeavyReserveAmmoAttribute();
	}
	else if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Special")))
	{
		return GetMaxSpecialReserveAmmoAttribute();
	}

	return FGameplayAttribute();
}

void UAmmoAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute,
	const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UAmmoAttributeSet::OnRep_NormalReserveAmmo(const FGameplayAttributeData& OldNormalReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, NormalReserveAmmo, OldNormalReserveAmmo);
}

void UAmmoAttributeSet::OnRep_MaxNormalReserveAmmo(const FGameplayAttributeData& OldMaxNormalReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, MaxNormalReserveAmmo, OldMaxNormalReserveAmmo);
}

void UAmmoAttributeSet::OnRep_HeavyReserveAmmo(const FGameplayAttributeData& OldHeavyReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, HeavyReserveAmmo, OldHeavyReserveAmmo);
}

void UAmmoAttributeSet::OnRep_MaxHeavyReserveAmmo(const FGameplayAttributeData& OldMaxHeavyReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, MaxHeavyReserveAmmo, OldMaxHeavyReserveAmmo);
}

void UAmmoAttributeSet::OnRep_SpecialReserveAmmo(const FGameplayAttributeData& OldSpecialReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, SpecialReserveAmmo, OldSpecialReserveAmmo);
}

void UAmmoAttributeSet::OnRep_MaxSpecialReserveAmmo(const FGameplayAttributeData& OldMaxSpecialReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, MaxSpecialReserveAmmo, OldMaxSpecialReserveAmmo);
}
