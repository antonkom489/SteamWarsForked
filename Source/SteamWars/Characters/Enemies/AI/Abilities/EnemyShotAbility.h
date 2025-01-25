#pragma once

#include "CoreMinimal.h"
#include "Components/CharacterComponents/AbilitySystem/Abilities/SWGameplayAbility.h"
#include "EnemyShotAbility.generated.h"

UCLASS()
class STEAMWARS_API UEnemyShotAbility : public USWGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyShotAbility();

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UGameplayEffect> ResponseGameplayEffect;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Damage = 30.0f;

private:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;	
	
};
