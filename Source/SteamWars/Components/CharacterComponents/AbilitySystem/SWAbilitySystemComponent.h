#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "SWAbilitySystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReceivedDamageDelegate, USWAbilitySystemComponent*, SourceASC, float, UnmitigatedDamage, float, MitigatedDamage);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEAMWARS_API USWAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	bool bCharacterAbilitiesGiven = false;
	bool bStartupEffectsApplied = false;

	FReceivedDamageDelegate ReceivedDamage;

	// Called from GDDamageExecCalculation. Broadcasts on ReceivedDamage whenever this ASC receives damage.
	virtual void ReceiveDamage(USWAbilitySystemComponent* SourceASC, float UnmitigatedDamage, float MitigatedDamage);
};
