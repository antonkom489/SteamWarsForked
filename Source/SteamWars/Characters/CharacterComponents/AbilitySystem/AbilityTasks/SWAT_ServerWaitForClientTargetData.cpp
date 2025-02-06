#include "SWAT_ServerWaitForClientTargetData.h"

#include "AbilitySystemComponent.h"

USWAT_ServerWaitForClientTargetData::USWAT_ServerWaitForClientTargetData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

USWAT_ServerWaitForClientTargetData* USWAT_ServerWaitForClientTargetData::ServerWaitForClientTargetData(UGameplayAbility* OwningAbility, FName TaskInstanceName, bool TriggerOnce)
{
	USWAT_ServerWaitForClientTargetData* MyObj = NewAbilityTask<USWAT_ServerWaitForClientTargetData>(OwningAbility, TaskInstanceName);
	MyObj->bTriggerOnce = TriggerOnce;
	return MyObj;
}

void USWAT_ServerWaitForClientTargetData::Activate()
{
	if (!Ability || !Ability->GetCurrentActorInfo()->IsNetAuthority())
	{
		return;
	}

	FGameplayAbilitySpecHandle	SpecHandle = GetAbilitySpecHandle();
	FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
	AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &USWAT_ServerWaitForClientTargetData::OnTargetDataReplicatedCallback);
}

void USWAT_ServerWaitForClientTargetData::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)
{
	FGameplayAbilityTargetDataHandle MutableData = Data;
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(MutableData);
	}

	if (bTriggerOnce)
	{
		EndTask();
	}
}

void USWAT_ServerWaitForClientTargetData::OnDestroy(bool AbilityEnded)
{
	if (AbilitySystemComponent.IsValid())
	{
		FGameplayAbilitySpecHandle	SpecHandle = GetAbilitySpecHandle();
		FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).RemoveAll(this);
	}

	Super::OnDestroy(AbilityEnded);
}