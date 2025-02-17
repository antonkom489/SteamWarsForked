#include "EnemyShotAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Characters/Enemies/EnemyBaseCharacter.h"

UEnemyShotAbility::UEnemyShotAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTag ShootTag = FGameplayTag::RequestGameplayTag(FName("AI.Enemy.ShotAbility"));
	AbilityTags.AddTag(ShootTag);
	ActivationOwnedTags.AddTag(ShootTag);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Enemy")));
}

void UEnemyShotAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ActorInfo->OwnerActor.IsValid())
	{
		AEnemyBaseCharacter* Hero = Cast<AEnemyBaseCharacter>(GetAvatarActorFromActorInfo());

		if (!Hero)
		{
			UE_LOG(LogTemp, Warning, TEXT("!Hero"))
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
			return;
		}
		
		FVector PVP = Hero->GetActorLocation();
		FRotator PVR = Hero->GetActorRotation();
		
		FVector ViewDirection = PVR.RotateVector(FVector::ForwardVector);

		FVector ShotStart = Hero->GetActorLocation() + FVector(0.0f, 0.f, 0.f);
		//FVector ShotStart = PVP + FVector(50.0f, 0, 50.0f);
		FVector ShotEnd = ShotStart + ViewDirection * 300;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Hero);
		
		TArray<FHitResult> ShotHitResults;

		if(GetWorld()->SweepMultiByChannel(ShotHitResults, ShotStart, ShotEnd, FQuat::Identity, ECC_Bullet, FCollisionShape::MakeSphere(40.0f)))
		{
			for(FHitResult HitResult : ShotHitResults)
			{
				ShotEnd = HitResult.Location;
				DrawDebugSphere(GetWorld(), ShotEnd, 50.f, 24, FColor::Red, false, 1.f);
				UE_LOG(LogTemp, Warning, TEXT("Hit!"))

				AActor* HitActor = HitResult.GetActor();
				if (HitActor && HitActor != Hero) // Проверяем, чтобы не наносить урон самому себе
				{
					if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor))
					{
						FGameplayEffectSpecHandle DamageEffectSpecHandle = ASC->MakeOutgoingSpec(
							ResponseGameplayEffect, 1, ASC->MakeEffectContext());
                    
						DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(
							FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
                
						ASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
					}
				}
			}
		}
		DrawDebugLine(GetWorld(), ShotStart, ShotEnd, FColor::Green, false, 1.f, 0, 0.3f);
		
		/*FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(ResponseGameplayEffect, GetAbilityLevel());
		
		// Pass the damage to the Damage Execution Calculation through a SetByCaller value on the GameplayEffectSpec
		DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);

		if(UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ShotHitResult.GetActor()))
		{
			DamageEffectSpecHandle = ASC->MakeOutgoingSpec(
						ResponseGameplayEffect, 1, ASC->MakeEffectContext());
			
			DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
		
			ASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}*/
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
