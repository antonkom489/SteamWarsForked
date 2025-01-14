#include "WeaponBarrelComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "SWTypes.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"


UWeaponBarrelComponent::UWeaponBarrelComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UWeaponBarrelComponent::Shot(FVector ShotStart, FVector ShotDirection)
{
	FVector ShotEnd = ShotStart + ShotDirection * FiringRange;

	FHitResult ShotHitResult;

	if(GetWorld()->LineTraceSingleByChannel(ShotHitResult, ShotStart, ShotEnd, ECC_Bullet))
	{
		ShotEnd = ShotHitResult.Location;
		DrawDebugSphere(GetWorld(), ShotEnd, 10.f, 24, FColor::Red, false, 1.f);
	}
	DrawDebugLine(GetWorld(), GetComponentLocation(), ShotEnd, FColor::Green, false, 1.f, 0, 0.3f);

	if(UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ShotHitResult.GetActor()))
	{
		FGameplayEffectSpecHandle DamageEffectSpecHandle = ASC->MakeOutgoingSpec(
					ResponseGameplayEffect, 1, ASC->MakeEffectContext());
		
		DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(FName("DamageData.Base")), Damage);
		
		ASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
	}
		
}

