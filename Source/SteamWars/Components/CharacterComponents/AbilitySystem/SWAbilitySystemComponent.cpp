#include "SWAbilitySystemComponent.h"

void USWAbilitySystemComponent::ReceiveDamage(USWAbilitySystemComponent* SourceASC, float UnmitigatedDamage,
	float MitigatedDamage)
{
	ReceivedDamage.Broadcast(SourceASC, UnmitigatedDamage, MitigatedDamage);
}

