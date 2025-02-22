#include "SWDamageable.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

bool IGSDamageable::IsAvailableForTakedown_Implementation(UPrimitiveComponent* TakedownComponent) const
{
	return false;
}

float IGSDamageable::GetTakedownDuration_Implementation(UPrimitiveComponent* TakedownComponent) const
{
	return 0.0f;
}

void IGSDamageable::GetPreTakedownSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* TakedownComponent) const
{
	bShouldSync = false;
	Type = EAbilityTaskNetSyncType::OnlyServerWait;
}

void IGSDamageable::GetPostTakedownSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* TakedownComponent) const
{
	bShouldSync = false;
	Type = EAbilityTaskNetSyncType::OnlyServerWait;
}

FSimpleMulticastDelegate* IGSDamageable::GetTargetCancelTakedownDelegate(UPrimitiveComponent* TakedownComponent)
{
	return nullptr;
}

void IGSDamageable::RegisterTakedowner_Implementation(UPrimitiveComponent* TakedownComponent, AActor* TakedowningActor)
{
	if (Takedowners.Contains(TakedownComponent))
	{
		TArray<AActor*>& TakedowningActors = Takedowners[TakedownComponent];
		if (!TakedowningActors.Contains(TakedowningActor))
		{
			TakedowningActors.Add(TakedowningActor);
		}
	}
	else
	{
		TArray<AActor*> TakedowningActors;
		TakedowningActors.Add(TakedowningActor);
		Takedowners.Add(TakedownComponent, TakedowningActors);
	}
}

void IGSDamageable::UnregisterTakedowner_Implementation(UPrimitiveComponent* TakedownComponent, AActor* TakedowningActor)
{
	if (Takedowners.Contains(TakedownComponent))
	{
		TArray<AActor*>& TakedowningActors = Takedowners[TakedownComponent];
		TakedowningActors.Remove(TakedowningActor);
	}
}

void IGSDamageable::TakedownableCancelTakedown_Implementation(UPrimitiveComponent* TakedownComponent)
{
	if (Takedowners.Contains(TakedownComponent))
	{
		FGameplayTagContainer TakedownAbilityTagContainer;
		TakedownAbilityTagContainer.AddTag(FGameplayTag::RequestGameplayTag("Ability.Takedown"));

		TArray<AActor*>& TakedowningActors = Takedowners[TakedownComponent];
		for (AActor* TakedowningActor : TakedowningActors)
		{
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TakedowningActor);

			if (ASC)
			{
				ASC->CancelAbilities(&TakedownAbilityTagContainer);
			}
		}

		TakedowningActors.Empty();
	}
}

bool IGSDamageable::IsStatusBarAvailable_Implementation() const
{
	return false;
}

void IGSDamageable::FadeInStatusBar_Implementation() const
{
	return;
}

void IGSDamageable::FadeOutStatusBar_Implementation() const
{
	return;
}