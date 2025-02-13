#include "SWGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Characters/SWBaseCharacter.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"
#include "Characters/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"
#include "Characters/CharacterComponents/AbilitySystem/SWAbilityTpes.h"
#include "Characters/CharacterComponents/AbilitySystem/SWTargetType.h"

USWGameplayAbility::USWGameplayAbility()
{
	// Default to Instance Per Actor
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Default tags that block this ability from activating
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Debuff.Stun")));
}

void USWGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (ActivateAbilityOnGranted)
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
	}
}

FGameplayAbilityTargetDataHandle USWGameplayAbility::MakeGameplayAbilityTargetDataHandleFromActorArray(
	const TArray<AActor*> TargetActors)
{
	if (TargetActors.Num() > 0)
	{
		FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
		NewData->TargetActorArray.Append(TargetActors);
		return FGameplayAbilityTargetDataHandle(NewData);
	}

	return FGameplayAbilityTargetDataHandle();
}

FGameplayAbilityTargetDataHandle USWGameplayAbility::MakeGameplayAbilityTargetDataHandleFromHitResults(
	const TArray<FHitResult> HitResults)
{
	FGameplayAbilityTargetDataHandle TargetData;

	for (const FHitResult& HitResult : HitResults)
	{
		FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
		TargetData.Add(NewData);
	}

	return TargetData;
}

FSWGameplayEffectContainerSpec USWGameplayAbility::MakeEffectContainerSpecFromContainer(
	const FSWGameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	// First figure out our actor info
	FSWGameplayEffectContainerSpec ReturnSpec;
	AActor* OwningActor = GetOwningActorFromActorInfo();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	ASWBaseCharacter* AvatarCharacter = Cast<ASWBaseCharacter>(AvatarActor);
	USWAbilitySystemComponent* OwningASC = USWAbilitySystemComponent::GetAbilitySystemComponentFromActor(OwningActor);

	if (OwningASC)
	{
		// If we have a target type, run the targeting logic. This is optional, targets can be added later
		if (Container.TargetType.Get())
		{
			TArray<FHitResult> HitResults;
			TArray<AActor*> TargetActors;
			TArray<FGameplayAbilityTargetDataHandle> TargetData;
			const USWTargetType* TargetTypeCDO = Container.TargetType.GetDefaultObject();
			TargetTypeCDO->GetTargets(AvatarCharacter, AvatarActor, EventData, TargetData, HitResults, TargetActors);
			ReturnSpec.AddTargets(TargetData, HitResults, TargetActors);
		}

		// If we don't have an override level, use the ability level
		if (OverrideGameplayLevel == INDEX_NONE)
		{
			//OverrideGameplayLevel = OwningASC->GetDefaultAbilityLevel();
			OverrideGameplayLevel = GetAbilityLevel();
		}

		// Build GameplayEffectSpecs for each applied effect
		for (const TSubclassOf<UGameplayEffect>& EffectClass : Container.TargetGameplayEffectClasses)
		{
			ReturnSpec.TargetGameplayEffectSpecs.Add(MakeOutgoingGameplayEffectSpec(EffectClass, OverrideGameplayLevel));
		}
	}
	return ReturnSpec;
}

FSWGameplayEffectContainerSpec USWGameplayAbility::MakeEffectContainerSpec(FGameplayTag ContainerTag,
	const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	FSWGameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

	if (FoundContainer)
	{
		return MakeEffectContainerSpecFromContainer(*FoundContainer, EventData, OverrideGameplayLevel);
	}
	return FSWGameplayEffectContainerSpec();
}

/*FSWGameplayEffectContainerSpec USWGameplayAbility::MakeEffectContainerSpecFromContainer(
	const FSWGameplayEffectContainerSpec& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	// First figure out our actor info
	FSWGameplayEffectContainerSpec ReturnSpec;
	AActor* OwningActor = GetOwningActorFromActorInfo();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	ASWBaseCharacter* AvatarCharacter = Cast<ASWBaseCharacter>(AvatarActor);
	USWAbilitySystemComponent* OwningASC = USWAbilitySystemComponent::GetAbilitySystemComponentFromActor(OwningActor);

	if (OwningASC)
	{
		// If we have a target type, run the targeting logic. This is optional, targets can be added later
		if (Container.TargetType.Get())
		{
			TArray<FHitResult> HitResults;
			TArray<AActor*> TargetActors;
			TArray<FGameplayAbilityTargetDataHandle> TargetData;
			const USWTargetType* TargetTypeCDO = Container.TargetType.GetDefaultObject();
			TargetTypeCDO->GetTargets(AvatarCharacter, AvatarActor, EventData, TargetData, HitResults, TargetActors);
			ReturnSpec.AddTargets(TargetData, HitResults, TargetActors);
		}

		// If we don't have an override level, use the ability level
		if (OverrideGameplayLevel == INDEX_NONE)
		{
			//OverrideGameplayLevel = OwningASC->GetDefaultAbilityLevel();
			OverrideGameplayLevel = GetAbilityLevel();
		}

		// Build GameplayEffectSpecs for each applied effect
		for (const TSubclassOf<UGameplayEffect>& EffectClass : Container.TargetGameplayEffectClasses)
		{
			ReturnSpec.TargetGameplayEffectSpecs.Add(MakeOutgoingGameplayEffectSpec(EffectClass, OverrideGameplayLevel));
		}
	}
	return ReturnSpec;
}*/

/*FSWGameplayEffectContainerSpec USWGameplayAbility::MakeEffectContainerSpec(FGameplayTag ContainerTag,
	const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	FSWGameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

	if (FoundContainer)
	{
		return MakeEffectContainerSpecFromContainer(*FoundContainer, EventData, OverrideGameplayLevel);
	}
	return FSWGameplayEffectContainerSpec();
}*/

TArray<FActiveGameplayEffectHandle> USWGameplayAbility::ApplyEffectContainerSpec(
	const FSWGameplayEffectContainerSpec& ContainerSpec)
{
	TArray<FActiveGameplayEffectHandle> AllEffects;

	// Iterate list of effect specs and apply them to their target data
	for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
		AllEffects.Append(K2_ApplyGameplayEffectSpecToTarget(SpecHandle, ContainerSpec.TargetData));
	}
	return AllEffects;
}

UObject* USWGameplayAbility::K2_GetSourceObject(FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo& ActorInfo) const
{
	return GetSourceObject(Handle, &ActorInfo);
}

bool USWGameplayAbility::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle,
	bool EndAbilityImmediately)
{
	USWAbilitySystemComponent* GSASC = Cast<USWAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (GSASC)
	{
		return GSASC->BatchRPCTryActivateAbility(InAbilityHandle, EndAbilityImmediately);
	}

	return false;
}

/*bool USWGameplayAbility::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle,
	bool EndAbilityImmediately)
{
	USWAbilitySystemComponent* GSASC = Cast<USWAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (GSASC)
	{
		return GSASC->BatchRPCTryActivateAbility(InAbilityHandle, EndAbilityImmediately);
	}

	return false;
}*/

void USWGameplayAbility::ExternalEndAbility()
{
	check(CurrentActorInfo);

	bool bReplicateEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FString USWGameplayAbility::GetCurrentPredictionKeyStatus()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC->ScopedPredictionKey.ToString() + " is valid for more prediction: " + (ASC->ScopedPredictionKey.IsValidForMorePrediction() ? TEXT("true") : TEXT("false"));
}

bool USWGameplayAbility::IsPredictionKeyValidForMorePrediction() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC->ScopedPredictionKey.IsValidForMorePrediction();
}

bool USWGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	/*if (bCannotActivateWhileInteracting)
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (ASC->GetTagCount(InteractingTag) > ASC->GetTagCount(InteractingRemovalTag))
		{
			return false;
		}
	}*/

	if (bSourceObjectMustEqualCurrentWeaponToActivate)
	{
		ASWFPSCharacter* Hero = Cast<ASWFPSCharacter>(ActorInfo->AvatarActor);
		if (Hero && Hero->GetCurrentWeapon() && (UObject*)Hero->GetCurrentWeapon() == GetSourceObject(Handle, ActorInfo))
		{
			return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
		}
		else
		{
			return false;
		}
	}

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

bool USWGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) && GSCheckCost(Handle, *ActorInfo);
}

bool USWGameplayAbility::GSCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo& ActorInfo) const
{
	return true;
}

void USWGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	GSApplyCost(Handle, *ActorInfo, ActivationInfo);
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void USWGameplayAbility::GSApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	return;
}

void USWGameplayAbility::SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetData)
{
	if (IsPredictingClient())
	{
		UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
		check(ASC);

		FScopedPredictionWindow	ScopedPrediction(ASC, IsPredictingClient());

		FGameplayTag ApplicationTag; // Fixme: where would this be useful?
		CurrentActorInfo->AbilitySystemComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle,
			CurrentActivationInfo.GetActivationPredictionKey(), TargetData, ApplicationTag, ASC->ScopedPredictionKey);
	}
}

bool USWGameplayAbility::IsInputPressed() const
{
	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	return Spec && Spec->InputPressed;
}

