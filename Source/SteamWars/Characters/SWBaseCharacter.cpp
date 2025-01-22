#include "SWBaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/CharacterComponents/SWCharacterEquipmentComponent.h"
#include "Components/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"
#include "Components/CharacterComponents/AbilitySystem/AttributeSet/SWAttributeSet.h"
#include "Components/CharacterComponents/AbilitySystem/Abilities/SWGameplayAbility.h"
#include "GameFramework/CharacterMovementComponent.h"

ASWBaseCharacter::ASWBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CharacterEquipmentComponent = CreateDefaultSubobject<USWCharacterEquipmentComponent>(TEXT("CharacterEquipment"));
	
	DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
	EffectRemoveOnDeathTag = FGameplayTag::RequestGameplayTag(FName("Effect.RemoveOnDeath"));

	/*HitDirectionFrontTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Front"));
	HitDirectionBackTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Back"));
	HitDirectionRightTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Right"));
	HitDirectionLeftTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Left"));*/
}

UAbilitySystemComponent* ASWBaseCharacter::GetAbilitySystemComponent() const
{
	if(AbilitySystemComponent.IsValid())
	{
		return AbilitySystemComponent.Get();
	}
	return nullptr;
}

USWAttributeSet* ASWBaseCharacter::GetAttributeSet() const
{
	if(AttributeSet.IsValid())
	{
		return AttributeSet.Get();
	}
	return nullptr;
}

int32 ASWBaseCharacter::GetAbilityLevel(ESWAbilityInputID AbilityID) const
{
	return 1;
}

void ASWBaseCharacter::RemoveCharacterAbilities()
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent.IsValid() || !AbilitySystemComponent->bCharacterAbilitiesGiven)
	{
		return;
	}
	
	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if ((Spec.SourceObject == this) && DefaultAbilities.Contains(Spec.Ability->GetClass()))
		{
			AbilitiesToRemove.Add(Spec.Handle);
		}
	}
	
	for (int32 i = 0; i < AbilitiesToRemove.Num(); i++)
	{
		AbilitySystemComponent->ClearAbility(AbilitiesToRemove[i]);
	}

	AbilitySystemComponent->bCharacterAbilitiesGiven = false;
}

bool ASWBaseCharacter::IsAlive() const
{
	return GetHealth() > 0.0f;
}

void ASWBaseCharacter::Die()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->GravityScale = 0;
	GetCharacterMovement()->Velocity = FVector(0);

	//OnCharacterDied.Broadcast(this);

	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;
		EffectTagsToRemove.AddTag(EffectRemoveOnDeathTag);
		int32 NumEffectsRemoved = AbilitySystemComponent->RemoveActiveEffectsWithTags(EffectTagsToRemove);

		AbilitySystemComponent->AddLooseGameplayTag(DeadTag);
	}

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
	
	FinishDying();
}

void ASWBaseCharacter::FinishDying()
{
	Destroy();
}

/*EGDHitReactDirection ASWBaseCharacter::GetHitReactDirection(const FVector& ImpactPoint)
{
	const FVector& ActorLocation = GetActorLocation();
	float DistanceToFrontBackPlane = FVector::PointPlaneDist(ImpactPoint, ActorLocation, GetActorRightVector());
	float DistanceToRightLeftPlane = FVector::PointPlaneDist(ImpactPoint, ActorLocation, GetActorForwardVector());
	
	if (FMath::Abs(DistanceToFrontBackPlane) <= FMath::Abs(DistanceToRightLeftPlane))
	{
		if (DistanceToRightLeftPlane >= 0)
		{
			return EGDHitReactDirection::Front;
		}
		else
		{
			return EGDHitReactDirection::Back;
		}
	}
	else
	{
		if (DistanceToFrontBackPlane >= 0)
		{
			return EGDHitReactDirection::Right;
		}
		else
		{
			return EGDHitReactDirection::Left;
		}
	}
}

void ASWBaseCharacter::PlayHitReact_Implementation(FGameplayTag HitDirection, AActor* DamageCauser)
{
	if (IsAlive())
	{
		if (HitDirection == HitDirectionLeftTag)
		{
			ShowHitReact.Broadcast(EGDHitReactDirection::Left);
		}
		else if (HitDirection == HitDirectionFrontTag)
		{
			ShowHitReact.Broadcast(EGDHitReactDirection::Front);
		}
		else if (HitDirection == HitDirectionRightTag)
		{
			ShowHitReact.Broadcast(EGDHitReactDirection::Right);
		}
		else if (HitDirection == HitDirectionBackTag)
		{
			ShowHitReact.Broadcast(EGDHitReactDirection::Back);
		}
	}
}

bool ASWBaseCharacter::PlayHitReact_Validate(FGameplayTag HitDirection, AActor* DamageCauser)
{
	return true;
}*/

int32 ASWBaseCharacter::GetCharacterLevel() const
{
	if (AttributeSet.IsValid())
	{
		return static_cast<int32>(AttributeSet->GetCharacterLevel());
	}

	return 0;
}

const USWCharacterEquipmentComponent* ASWBaseCharacter::GetEquipmentComponent() const
{
	if(CharacterEquipmentComponent)
	{
		return CharacterEquipmentComponent;
	}
	return nullptr;
}

float ASWBaseCharacter::GetHealth() const
{
	if(AttributeSet.IsValid())
	{
		return AttributeSet->GetHealth();
	}
	return 0.0f;
}

float ASWBaseCharacter::GetMaxHealth() const
{
	if(AttributeSet.IsValid())
	{
		return AttributeSet->GetMaxHealth();
	}
	return 0.0f;
}

float ASWBaseCharacter::GetStamina() const
{
	if(AttributeSet.IsValid())
	{
		return AttributeSet->GetStamina();
	}
	return 0.0f;
}

float ASWBaseCharacter::GetMaxStamina() const
{
	if(AttributeSet.IsValid())
	{
		return AttributeSet->GetMaxStamina();
	}
	return 0.0f;
}

void ASWBaseCharacter::GiveDefaultAbilities()
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent.IsValid() || AbilitySystemComponent->bCharacterAbilitiesGiven)
	{
		return;
	}

	for (TSubclassOf<USWGameplayAbility>& StartupAbility : DefaultAbilities)
	{
		AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(StartupAbility, GetAbilityLevel(StartupAbility.GetDefaultObject()->AbilityID),
				static_cast<int32>(StartupAbility.GetDefaultObject()->AbilityInputID), this));
	}

	AbilitySystemComponent->bCharacterAbilitiesGiven = true;
}

void ASWBaseCharacter::InitDefaultAttributes() const
{
	if (!AbilitySystemComponent.IsValid())
	{
		return;
	}

	if (!DefaultAttributeEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing DefaultAttributes for %s. Please fill in the character's Blueprint."), *FString(__FUNCTION__), *GetName());
		return;
	}

	// Can run on Server and Client
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, GetCharacterLevel(), EffectContext);
	if (NewHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent.Get());
	}
}

void ASWBaseCharacter::AddStartupEffects()
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent.IsValid() || AbilitySystemComponent->bStartupEffectsApplied)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (TSubclassOf<UGameplayEffect> GameplayEffect : StartupEffects)
	{
		FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, GetCharacterLevel(), EffectContext);
		if (NewHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent.Get());
		}
	}

	AbilitySystemComponent->bStartupEffectsApplied = true;
}

void ASWBaseCharacter::SetHealth(float Health)
{
	if(AttributeSet.IsValid())
	{
		AttributeSet->SetHealth(Health);
	}
}

void ASWBaseCharacter::SetStamina(float Stamina)
{
	if(AttributeSet.IsValid())
	{
		AttributeSet->SetStamina(Stamina);
	}
}


