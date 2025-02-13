// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SWTargetType.generated.h"

class ASWBaseCharacter;
/**
 * 
 */
UCLASS()
class STEAMWARS_API USWTargetType : public UObject
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	USWTargetType() {}

	/** Called to determine targets to apply gameplay effects to */
	UFUNCTION(BlueprintNativeEvent)
	void GetTargets(ASWBaseCharacter* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
	virtual void GetTargets_Implementation(ASWBaseCharacter* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
};

/** Trivial target type that uses the owner */
UCLASS(NotBlueprintable)
class STEAMWARS_API USWTargetType_UseOwner : public USWTargetType
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	USWTargetType_UseOwner() {}

	/** Uses the passed in event data */
	virtual void GetTargets_Implementation(ASWBaseCharacter* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};

/** Trivial target type that pulls the target out of the event data */
UCLASS(NotBlueprintable)
class STEAMWARS_API USWTargetType_UseEventData : public USWTargetType
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	USWTargetType_UseEventData() {}

	/** Uses the passed in event data */
	virtual void GetTargets_Implementation(ASWBaseCharacter* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};

