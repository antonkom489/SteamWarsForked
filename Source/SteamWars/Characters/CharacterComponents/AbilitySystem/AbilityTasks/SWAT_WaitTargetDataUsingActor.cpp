#include "SWAT_WaitTargetDataUsingActor.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Characters/CharacterComponents/AbilitySystem/Abilities/SWGATA_Trace.h"

USWAT_WaitTargetDataUsingActor::USWAT_WaitTargetDataUsingActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

USWAT_WaitTargetDataUsingActor* USWAT_WaitTargetDataUsingActor::WaitTargetDataWithReusableActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor, bool bCreateKeyIfNotValidForMorePredicting)
{
	USWAT_WaitTargetDataUsingActor* MyObj = NewAbilityTask<USWAT_WaitTargetDataUsingActor>(OwningAbility, TaskInstanceName);		//Register for task list here, providing a given FName as a key
	MyObj->TargetActor = InTargetActor;
	MyObj->ConfirmationType = ConfirmationType;
	MyObj->bCreateKeyIfNotValidForMorePredicting = bCreateKeyIfNotValidForMorePredicting;
	return MyObj;
}

void USWAT_WaitTargetDataUsingActor::Activate()
{
	//if (IsPendingKill())
	if (!IsValidChecked(this))
	{
		return;
	}

	if (Ability && TargetActor)
	{
		InitializeTargetActor();
		RegisterTargetDataCallbacks();
		FinalizeTargetActor();
	}
	else
	{
		EndTask();
	}
}

void USWAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)
{
	//check(AbilitySystemComponent);

	FGameplayAbilityTargetDataHandle MutableData = Data;
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	/**
	 *  Call into the TargetActor to sanitize/verify the data. If this returns false, we are rejecting
	 *	the replicated target data and will treat this as a cancel.
	 *
	 *	This can also be used for bandwidth optimizations. OnReplicatedTargetDataReceived could do an actual
	 *	trace/check/whatever server side and use that data. So rather than having the client send that data
	 *	explicitly, the client is basically just sending a 'confirm' and the server is now going to do the work
	 *	in OnReplicatedTargetDataReceived.
	 */
	if (TargetActor && !TargetActor->OnReplicatedTargetDataReceived(MutableData))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			Cancelled.Broadcast(MutableData);
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			ValidData.Broadcast(MutableData);
		}
	}

	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

void USWAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCancelledCallback()
{
	//check(AbilitySystemComponent);
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
	}
	EndTask();
}

void USWAT_WaitTargetDataUsingActor::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data)
{
	//check(AbilitySystemComponent);
	if (!Ability)
	{
		return;
	}

	FScopedPredictionWindow	ScopedPrediction(AbilitySystemComponent.Get(),
		ShouldReplicateDataToServer() && (bCreateKeyIfNotValidForMorePredicting && !AbilitySystemComponent.Get()->ScopedPredictionKey.IsValidForMorePrediction()));

	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (IsPredictingClient())
	{
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			FGameplayTag ApplicationTag; // Fixme: where would this be useful?
			AbilitySystemComponent->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey(), Data, ApplicationTag, AbilitySystemComponent->ScopedPredictionKey);
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			// We aren't going to send the target data, but we will send a generic confirmed message.
			AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
		}
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(Data);
	}

	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

void USWAT_WaitTargetDataUsingActor::OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data)
{
	//check(AbilitySystemComponent);

	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get(), IsPredictingClient());

	if (IsPredictingClient())
	{
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			AbilitySystemComponent.Get()->ServerSetReplicatedTargetDataCancelled(GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent.Get()->ScopedPredictionKey);
		}
		else
		{
			// We aren't going to send the target data, but we will send a generic confirmed message.
			AbilitySystemComponent.Get()->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent.Get()->ScopedPredictionKey);
		}
	}
	Cancelled.Broadcast(Data);
	EndTask();
}

void USWAT_WaitTargetDataUsingActor::ExternalConfirm(bool bEndTask)
{
	//check(AbilitySystemComponent);
	if (TargetActor)
	{
		if (TargetActor->ShouldProduceTargetData())
		{
			TargetActor->ConfirmTargetingAndContinue();
		}
	}
	Super::ExternalConfirm(bEndTask);
}

void USWAT_WaitTargetDataUsingActor::ExternalCancel()
{
	//check(AbilitySystemComponent);
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
	}
	Super::ExternalCancel();
}

void USWAT_WaitTargetDataUsingActor::InitializeTargetActor() const
{
	check(TargetActor);
	check(Ability);

	TargetActor->PrimaryPC = Ability->GetCurrentActorInfo()->PlayerController.Get();

	// If we spawned the target actor, always register the callbacks for when the data is ready.
	TargetActor->TargetDataReadyDelegate.AddUObject(const_cast<USWAT_WaitTargetDataUsingActor*>(this), &USWAT_WaitTargetDataUsingActor::OnTargetDataReadyCallback);
	TargetActor->CanceledDelegate.AddUObject(const_cast<USWAT_WaitTargetDataUsingActor*>(this), &USWAT_WaitTargetDataUsingActor::OnTargetDataCancelledCallback);
}

void USWAT_WaitTargetDataUsingActor::FinalizeTargetActor() const
{
	check(TargetActor);
	check(Ability);

	TargetActor->StartTargeting(Ability);

	if (TargetActor->ShouldProduceTargetData())
	{
		// If instant confirm, then stop targeting immediately.
		// Note this is kind of bad: we should be able to just call a static func on the CDO to do this. 
		// But then we wouldn't get to set ExposeOnSpawnParameters.
		if (ConfirmationType == EGameplayTargetingConfirmation::Instant)
		{
			TargetActor->ConfirmTargeting();
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			// Bind to the Cancel/Confirm Delegates (called from local confirm or from repped confirm)
			TargetActor->BindToConfirmCancelInputs();
		}
	}
}

void USWAT_WaitTargetDataUsingActor::RegisterTargetDataCallbacks()
{
	//if (!ensure(IsPendingKill() == false))
	if (!IsValidChecked(this))
	{
		return;
	}

	check(Ability);

	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	const bool bShouldProduceTargetDataOnServer = TargetActor->ShouldProduceTargetDataOnServer;

	// If not locally controlled (server for remote client), see if TargetData was already sent
	// else register callback for when it does get here.
	if (!bIsLocallyControlled)
	{
		// Register with the TargetData callbacks if we are expecting client to send them
		if (!bShouldProduceTargetDataOnServer)
		{
			FGameplayAbilitySpecHandle	SpecHandle = GetAbilitySpecHandle();
			FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

			//Since multifire is supported, we still need to hook up the callbacks
			AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &USWAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCallback);
			AbilitySystemComponent->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &USWAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCancelledCallback);

			AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

			SetWaitingOnRemotePlayerData();
		}
	}
}

void USWAT_WaitTargetDataUsingActor::OnDestroy(bool AbilityEnded)
{
	if (TargetActor)
	{
		ASWGATA_Trace* TraceTargetActor = Cast<ASWGATA_Trace>(TargetActor);
		if (TraceTargetActor)
		{
			TraceTargetActor->StopTargeting();
		}
		else
		{
			// TargetActor doesn't have a StopTargeting function
			TargetActor->SetActorTickEnabled(false);

			// Clear added callbacks
			TargetActor->TargetDataReadyDelegate.RemoveAll(this);
			TargetActor->CanceledDelegate.RemoveAll(this);

			AbilitySystemComponent->GenericLocalConfirmCallbacks.RemoveDynamic(TargetActor, &AGameplayAbilityTargetActor::ConfirmTargeting);
			AbilitySystemComponent->GenericLocalCancelCallbacks.RemoveDynamic(TargetActor, &AGameplayAbilityTargetActor::CancelTargeting);
			TargetActor->GenericDelegateBoundASC = nullptr;
		}
	}

	Super::OnDestroy(AbilityEnded);
}

bool USWAT_WaitTargetDataUsingActor::ShouldReplicateDataToServer() const
{
	if (!Ability || !TargetActor)
	{
		return false;
	}

	// Send TargetData to the server IFF we are the client and this isn't a GameplayTargetActor that can produce data on the server	
	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (!Info->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer)
	{
		return true;
	}

	return false;
}