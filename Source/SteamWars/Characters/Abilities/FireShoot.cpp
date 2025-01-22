#include "FireShoot.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Characters/Enemies/EnemyBaseCharacter.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"
#include "Components/CharacterComponents/SWCharacterEquipmentComponent.h"
#include "Components/CharacterComponents/AbilitySystem/AbilityTasks/SW_PlayMontageAndWaitForEvent.h"
#include "Components/Weapon/WeaponBarrelComponent.h"

UFireShoot::UFireShoot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTag ShootTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Hit"));
	AbilityTags.AddTag(ShootTag);
	ActivationOwnedTags.AddTag(ShootTag);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon")));
}

void UFireShoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}

	UAnimMontage* MontageToPlay = FireHipMontage;

	if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.AimDownSights"))) &&
		!GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.AimDownSights.Removal"))))
	{
		MontageToPlay = FireIronsightsMontage;
	}
	

	// Only spawn projectiles on the Server.
	// Predicting projectiles is an advanced topic not covered in this example.
	if (GetOwningActorFromActorInfo()->GetLocalRole() == ROLE_Authority)
	{
		ASWFPSCharacter* Hero = Cast<ASWFPSCharacter>(GetAvatarActorFromActorInfo());
		if (!Hero)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}

		APlayerController* Controller = Hero->GetController<APlayerController>();
		if(!IsValid(Controller)) return;

		FVector PlayerViewPoint;
		FRotator PlayerViewRotation;
		
		Controller->GetPlayerViewPoint(PlayerViewPoint, PlayerViewRotation);

		FVector ViewDirection = PlayerViewRotation.RotateVector(FVector::ForwardVector);

		Range = Hero->GetEquipmentComponent()->GetCurrentWeapon()->GetWeaponBarrelComponent()->GetFiringRange();
		Damage = Hero->GetEquipmentComponent()->GetCurrentWeapon()->GetWeaponBarrelComponent()->GetDamage();
		Threat = Hero->GetEquipmentComponent()->GetCurrentWeapon()->GetWeaponBarrelComponent()->GetThreat();

		FVector ShotStart = Hero->GetEquipmentComponent()->GetCurrentWeapon()->GetWeaponBarrelComponent()->GetComponentLocation();
		//FVector ShotStart = PlayerViewPoint;
		FVector ShotEnd = PlayerViewPoint + ViewDirection * Range;
		FVector ShotEndThreat = PlayerViewPoint + ViewDirection * Range;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Hero);

		FHitResult ShotHitResult;

		if(GetWorld()->LineTraceSingleByChannel(ShotHitResult, ShotStart, ShotEnd, ECC_Bullet))
		{
			ShotEnd = ShotHitResult.Location;
			DrawDebugSphere(GetWorld(), ShotEnd, 10.f, 24, FColor::Red, false, 1.f);
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
		
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	/*Task = USW_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(this, NAME_None, MontageToPlay, FGameplayTagContainer(), 1.0f, NAME_None, false, 1.0f);
	Task->OnBlendOut.AddDynamic(this, &UFireShoot::EventReceived); //OnCompleted
	Task->OnCompleted.AddDynamic(this, &UFireShoot::EventReceived); //OnCompleted
	Task->OnInterrupted.AddDynamic(this, &UFireShoot::EventReceived); //OnCancelled
	Task->OnCancelled.AddDynamic(this, &UFireShoot::EventReceived); //OnCancelled
	Task->EventReceived.AddDynamic(this, &UFireShoot::EventReceived);
	// ReadyForActivation() is how you activate the AbilityTask in C++. Blueprint has magic from K2Node_LatentGameplayTaskCall that will automatically call ReadyForActivation().
	Task->ReadyForActivation();*/
}

void UFireShoot::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	UE_LOG(LogTemp, Warning, TEXT("EndAbility!"));
	if(Task.IsValid())
	{
		Task->ExternalCancel();
	}
}

void UFireShoot::OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCancelled!"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UFireShoot::EventReceived(FGameplayTag EventTag, FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("EventReceived!"));
	if (EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.EndAbility")))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// Only spawn projectiles on the Server.
	// Predicting projectiles is an advanced topic not covered in this example.
	if (GetOwningActorFromActorInfo()->GetLocalRole() == ROLE_Authority && EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.SpawnProjectile")))
	{
		ASWFPSCharacter* Hero = Cast<ASWFPSCharacter>(GetAvatarActorFromActorInfo());
		if (!Hero)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}

		APlayerController* Controller = Hero->GetController<APlayerController>();
		if(!IsValid(Controller)) return;

		FVector PlayerViewPoint;
		FRotator PlayerViewRotation;
		
		Controller->GetPlayerViewPoint(PlayerViewPoint, PlayerViewRotation);

		FVector ViewDirection = PlayerViewRotation.RotateVector(FVector::ForwardVector);

		Range = Hero->GetEquipmentComponent()->GetCurrentWeapon()->GetWeaponBarrelComponent()->GetFiringRange();
		Damage = Hero->GetEquipmentComponent()->GetCurrentWeapon()->GetWeaponBarrelComponent()->GetDamage();
		
		FVector ShotStart = Hero->GetEquipmentComponent()->GetCurrentWeapon()->GetWeaponBarrelComponent()->GetRelativeLocation();
		FVector ShotEnd = PlayerViewPoint + ViewDirection * Range;

		FHitResult ShotHitResult;

		if(GetWorld()->LineTraceSingleByChannel(ShotHitResult, ShotStart, ShotEnd, ECC_Bullet))
		{
			ShotEnd = ShotHitResult.Location;
			DrawDebugSphere(GetWorld(), ShotEnd, 10.f, 24, FColor::Red, false, 1.f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("NotLineTrace!"));
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
				FGameplayTag::RequestGameplayTag(FName("DamageData.Base")), Damage);
		
			ASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}
		
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UFireShoot::OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCompleted!"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
