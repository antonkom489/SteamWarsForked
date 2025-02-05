// Fill out your copyright notice in the Description page of Project Settings.


#include "SWAmmoAtributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

USWAmmoAtributeSet::USWAmmoAtributeSet()
{
	NormalAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Normal"));
	HeavyAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Heavy"));
	SpecialAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Special"));
}

void USWAmmoAtributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void USWAmmoAtributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
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

void USWAmmoAtributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USWAmmoAtributeSet, NormalReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USWAmmoAtributeSet, MaxNormalReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USWAmmoAtributeSet, HeavyReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USWAmmoAtributeSet, MaxHeavyReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USWAmmoAtributeSet, SpecialReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USWAmmoAtributeSet, MaxSpecialReserveAmmo, COND_None, REPNOTIFY_Always);
}

FGameplayAttribute USWAmmoAtributeSet::GetReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag)
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

FGameplayAttribute USWAmmoAtributeSet::GetMaxReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag)
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

void USWAmmoAtributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute,
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

void USWAmmoAtributeSet::OnRep_NormalReserveAmmo(const FGameplayAttributeData& OldNormalReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USWAmmoAtributeSet, NormalReserveAmmo, OldNormalReserveAmmo);
}

void USWAmmoAtributeSet::OnRep_MaxNormalReserveAmmo(const FGameplayAttributeData& OldMaxNormalReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USWAmmoAtributeSet, MaxNormalReserveAmmo, OldMaxNormalReserveAmmo);
}

void USWAmmoAtributeSet::OnRep_HeavyReserveAmmo(const FGameplayAttributeData& OldHeavyReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USWAmmoAtributeSet, HeavyReserveAmmo, OldHeavyReserveAmmo);
}

void USWAmmoAtributeSet::OnRep_MaxHeavyReserveAmmo(const FGameplayAttributeData& OldMaxHeavyReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USWAmmoAtributeSet, MaxHeavyReserveAmmo, OldMaxHeavyReserveAmmo);
}

void USWAmmoAtributeSet::OnRep_SpecialReserveAmmo(const FGameplayAttributeData& OldSpecialReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USWAmmoAtributeSet, SpecialReserveAmmo, OldSpecialReserveAmmo);
}

void USWAmmoAtributeSet::OnRep_MaxSpecialReserveAmmo(const FGameplayAttributeData& OldMaxSpecialReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USWAmmoAtributeSet, MaxSpecialReserveAmmo, OldMaxSpecialReserveAmmo);
}
