#include "EnemyShootAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Characters/Enemies/EnemyBaseCharacter.h"
#include "Characters/CharacterComponents/SWCharacterEquipmentComponent.h"
#include "Components/Weapon/WeaponBarrelComponent.h"


UEnemyShootAbility::UEnemyShootAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTag ShootTag = FGameplayTag::RequestGameplayTag(FName("AI.Enemy.ShootAbility"));
	AbilityTags.AddTag(ShootTag);
	ActivationOwnedTags.AddTag(ShootTag);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Enemy")));
}

void UEnemyShootAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	/*if (ActorInfo->OwnerActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("BAMAMMA!"))
		// Реализуйте стрельбу
		AEnemyBaseCharacter* Hero = Cast<AEnemyBaseCharacter>(GetAvatarActorFromActorInfo());

		if (!Hero)
		{
			UE_LOG(LogTemp, Warning, TEXT("!Hero"))
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}

		Range = Hero->GetEquipmentComponent()->GetCurrentWeapon()->GetWeaponBarrelComponent()->GetFiringRange();
		Damage = Hero->GetEquipmentComponent()->GetCurrentWeapon()->GetWeaponBarrelComponent()->GetDamage();

		
		FVector PVP = Hero->GetActorLocation();
		FRotator PVR = Hero->GetActorRotation();
		
		FVector ViewDirection = PVR.RotateVector(FVector::ForwardVector);

		FVector ShotStart = Hero->GetEquipmentComponent()->GetCurrentWeapon()->GetWeaponBarrelComponent()->GetComponentLocation();
		//FVector ShotStart = PVP + FVector(50.0f, 0, 50.0f);
		FVector ShotEnd = ShotStart + ViewDirection * 10000;

		FHitResult ShotHitResult;

		if(GetWorld()->LineTraceSingleByChannel(ShotHitResult, ShotStart, ShotEnd, ECC_Bullet))
		{
			ShotEnd = ShotHitResult.Location;
			DrawDebugSphere(GetWorld(), ShotEnd, 50.f, 24, FColor::Red, false, 1.f);
			UE_LOG(LogTemp, Warning, TEXT("Hit!"))
		}
		DrawDebugLine(GetWorld(), ShotStart, ShotEnd, FColor::Green, false, 1.f, 0, 0.3f);
		
		FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(ResponseGameplayEffect, GetAbilityLevel());
		
		// Pass the damage to the Damage Execution Calculation through a SetByCaller value on the GameplayEffectSpec
		DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);

		if(UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ShotHitResult.GetActor()))
		{
			DamageEffectSpecHandle = ASC->MakeOutgoingSpec(
						ResponseGameplayEffect, 1, ASC->MakeEffectContext());
			
			DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
		
			ASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}
	}*/
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
