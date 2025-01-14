#pragma once

#include "CoreMinimal.h"
#include "Components/CharacterComponents/AbilitySystem/Abilities/SWGameplayAbility.h"
#include "FireShoot.generated.h"

class USW_PlayMontageAndWaitForEvent;
class UAbilityTask_SuccessFailEvent;

UCLASS()
class STEAMWARS_API UFireShoot : public USWGameplayAbility
{
	GENERATED_BODY()

public:
	UFireShoot();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UAnimMontage* FireHipMontage;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UAnimMontage* FireIronsightsMontage;
	
protected:
	UFUNCTION()
	void OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void EventReceived(FGameplayTag EventTag, FGameplayEventData Payload);

	UFUNCTION()
	void OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UGameplayEffect> ResponseGameplayEffect;


private:
	TWeakObjectPtr<USW_PlayMontageAndWaitForEvent> Task;
	
	float Range;
	float Damage;
	
};
