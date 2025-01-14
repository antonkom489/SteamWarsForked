#pragma once

#include "CoreMinimal.h"
#include "../SWBaseCharacter.h"
#include "EnemyBaseCharacter.generated.h"

struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDied);

UCLASS()
class STEAMWARS_API AEnemyBaseCharacter : public ASWBaseCharacter
{
	GENERATED_BODY()

public:
	AEnemyBaseCharacter();
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyDied OnEnemyDied;
	
	virtual void FinishDying() override;
	
protected:
	virtual void BeginPlay() override;
	
	// Actual hard pointer to AbilitySystemComponent
	UPROPERTY()
	class USWAbilitySystemComponent* HardRefAbilitySystemComponent;

	// Actual hard pointer to AttributeSetBase
	UPROPERTY()
	class USWAttributeSet* HardRefAttributeSet;
	
	FDelegateHandle HealthChangedDelegateHandle;

	// Attribute changed callbacks
	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	// Tag change callbacks
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
};
