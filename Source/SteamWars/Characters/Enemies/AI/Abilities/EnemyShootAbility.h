#pragma once

#include "CoreMinimal.h"
#include "Components/CharacterComponents/AbilitySystem/Abilities/SWGameplayAbility.h"
#include "EnemyShootAbility.generated.h"

UCLASS()
class STEAMWARS_API UEnemyShootAbility : public USWGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyShootAbility();
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UGameplayEffect> ResponseGameplayEffect;

private:
	float Damage;
	float Range;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
