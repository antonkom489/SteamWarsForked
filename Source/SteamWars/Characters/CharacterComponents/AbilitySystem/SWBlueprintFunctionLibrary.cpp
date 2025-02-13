// Fill out your copyright notice in the Description page of Project Settings.


#include "SWBlueprintFunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "SWAbilityTpes.h"
#include "SWGameplayEffectTypes.h"
#include "Abilities/SWGameplayAbility.h"

FString USWBlueprintFunctionLibrary::GetPlayerEditorWindowRole(UWorld* World)
{
	FString Prefix;
	if (World)
	{
		if (World->WorldType == EWorldType::PIE)
		{
			switch (World->GetNetMode())
			{
			case NM_Client:
				Prefix = FString::Printf(TEXT("Client %d "), GPlayInEditorID - 1);
				break;
			case NM_DedicatedServer:
			case NM_ListenServer:
				Prefix = FString::Printf(TEXT("Server "));
				break;
			case NM_Standalone:
				break;
			}
		}
	}

	return Prefix;
}

USWGameplayAbility* USWBlueprintFunctionLibrary::GetPrimaryAbilityInstanceFromHandle(
	UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle)
{
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		if (AbilitySpec)
		{
			return Cast<USWGameplayAbility>(AbilitySpec->GetPrimaryInstance());
		}
	}

	return nullptr;
}

USWGameplayAbility* USWBlueprintFunctionLibrary::GetPrimaryAbilityInstanceFromClass(
	UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> InAbilityClass)
{
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(InAbilityClass);
		if (AbilitySpec)
		{
			return Cast<USWGameplayAbility>(AbilitySpec->GetPrimaryInstance());
		}
	}

	return nullptr;
}

bool USWBlueprintFunctionLibrary::IsPrimaryAbilityInstanceActive(UAbilitySystemComponent* AbilitySystemComponent,
	FGameplayAbilitySpecHandle Handle)
{
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		if (AbilitySpec)
		{
			return Cast<USWGameplayAbility>(AbilitySpec->GetPrimaryInstance())->IsActive();
		}
	}

	return false;
}

bool USWBlueprintFunctionLibrary::IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle)
{
	return Handle.IsValid();
}

bool USWBlueprintFunctionLibrary::DoesEffectContainerSpecHaveEffects(
	const FSWGameplayEffectContainerSpec& ContainerSpec)
{
	return ContainerSpec.HasValidEffects();
}

bool USWBlueprintFunctionLibrary::DoesEffectContainerSpecHaveTargets(
	const FSWGameplayEffectContainerSpec& ContainerSpec)
{
	return ContainerSpec.HasValidTargets();
}

void USWBlueprintFunctionLibrary::ClearEffectContainerSpecTargets(FSWGameplayEffectContainerSpec& ContainerSpec)
{
	ContainerSpec.ClearTargets();
}

void USWBlueprintFunctionLibrary::AddTargetsToEffectContainerSpec(FSWGameplayEffectContainerSpec& ContainerSpec,
	const TArray<FGameplayAbilityTargetDataHandle>& TargetData, const TArray<FHitResult>& HitResults,
	const TArray<AActor*>& TargetActors)
{
	ContainerSpec.AddTargets(TargetData, HitResults, TargetActors);
}

TArray<FActiveGameplayEffectHandle> USWBlueprintFunctionLibrary::ApplyExternalEffectContainerSpec(
	const FSWGameplayEffectContainerSpec& ContainerSpec)
{
	TArray<FActiveGameplayEffectHandle> AllEffects;

	// Iterate list of gameplay effects
	for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
		if (SpecHandle.IsValid())
		{
			// If effect is valid, iterate list of targets and apply to all
			for (TSharedPtr<FGameplayAbilityTargetData> Data : ContainerSpec.TargetData.Data)
			{
				AllEffects.Append(Data->ApplyGameplayEffectSpec(*SpecHandle.Data.Get()));
			}
		}
	}
	return AllEffects;
}

FGameplayAbilityTargetDataHandle USWBlueprintFunctionLibrary::EffectContextGetTargetData(
	FGameplayEffectContextHandle EffectContextHandle)
{
	FSWGameplayEffectContext* EffectContext = static_cast<FSWGameplayEffectContext*>(EffectContextHandle.Get());
	
	if (EffectContext)
	{
		return EffectContext->GetTargetData();
	}

	return FGameplayAbilityTargetDataHandle();
}

void USWBlueprintFunctionLibrary::EffectContextAddTargetData(FGameplayEffectContextHandle EffectContextHandle,
	const FGameplayAbilityTargetDataHandle& TargetData)
{
	FSWGameplayEffectContext* EffectContext = static_cast<FSWGameplayEffectContext*>(EffectContextHandle.Get());

	if (EffectContext)
	{
		EffectContext->AddTargetData(TargetData);
	}
}

void USWBlueprintFunctionLibrary::ClearTargetData(FGameplayAbilityTargetDataHandle& TargetData)
{
	TargetData.Clear();
}
