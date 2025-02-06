#include "SWBaseCharacter.h"
#include "CharacterComponents/SWCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Characters/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"
#include "Characters/CharacterComponents/AbilitySystem/AttributeSet/SWAttributeSet.h"
#include "Characters/CharacterComponents/AbilitySystem/Abilities/SWGameplayAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/SWFloatingStatusBarWidget.h"

ASWBaseCharacter::ASWBaseCharacter(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<USWCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	bAlwaysRelevant = true;

	// Cache tags
	HitDirectionFrontTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Front"));
	HitDirectionBackTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Back"));
	HitDirectionRightTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Right"));
	HitDirectionLeftTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Left"));
	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	BeingTakendownTag = FGameplayTag::RequestGameplayTag("State.BeingTakendown");
	EffectRemoveOnDeathTag = FGameplayTag::RequestGameplayTag("Effect.RemoveOnDeath");
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
	// Only runs on Server
	RemoveCharacterAbilities();
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->GravityScale = 0;
	GetCharacterMovement()->Velocity = FVector(0);

	OnCharacterDied.Broadcast(this);

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
	else
	{
		FinishDying();
	}
}

void ASWBaseCharacter::FinishDying()
{
	Destroy();
}

int32 ASWBaseCharacter::GetCharacterLevel() const
{
	if (AttributeSet.IsValid())
	{
		return static_cast<int32>(AttributeSet->GetCharacterLevel());
	}

	return 0;
}

ESWHitReactDirection ASWBaseCharacter::GetHitReactDirection(const FVector& ImpactPoint)
{
	/*const FVector& ActorLocation = GetActorLocation();
	// PointPlaneDist is super cheap - 1 vector subtraction, 1 dot product.
	float DistanceToFrontBackPlane = FVector::PointPlaneDist(ImpactPoint, ActorLocation, GetActorRightVector());
	float DistanceToRightLeftPlane = FVector::PointPlaneDist(ImpactPoint, ActorLocation, GetActorForwardVector());


	if (FMath::Abs(DistanceToFrontBackPlane) <= FMath::Abs(DistanceToRightLeftPlane))
	{
		// Determine if Front or Back

		// Can see if it's left or right of Left/Right plane which would determine Front or Back
		if (DistanceToRightLeftPlane >= 0)
		{
			return ESWHitReactDirection::Front;
		}
		else
		{
			return ESWHitReactDirection::Back;
		}
	}
	else
	{
		// Determine if Right or Left

		if (DistanceToFrontBackPlane >= 0)
		{
			return ESWHitReactDirection::Right;
		}
		else
		{
			return ESWHitReactDirection::Left;
		}
	}*/

	return ESWHitReactDirection::Front;
}

void ASWBaseCharacter::PlayHitReact_Implementation(FGameplayTag HitDirection, AActor* DamageCauser)
{
	if (IsAlive())
	{
		if (HitDirection == HitDirectionLeftTag)
		{
			ShowHitReact.Broadcast(ESWHitReactDirection::Left);
		}
		else if (HitDirection == HitDirectionFrontTag)
		{
			ShowHitReact.Broadcast(ESWHitReactDirection::Front);
		}
		else if (HitDirection == HitDirectionRightTag)
		{
			ShowHitReact.Broadcast(ESWHitReactDirection::Right);
		}
		else if (HitDirection == HitDirectionBackTag)
		{
			ShowHitReact.Broadcast(ESWHitReactDirection::Back);
		}
	}
}

bool ASWBaseCharacter::PlayHitReact_Validate(FGameplayTag HitDirection, AActor* DamageCauser)
{
	return true;
}

bool ASWBaseCharacter::IsAvailableForTakedown_Implementation(UPrimitiveComponent* TakedownComponent) const
{
	// Pawn is available to be takendown if HP is less than 25% and is not already being takendown.
	const float HPRatio = GetHealth() / GetMaxHealth();
	if (
		AbilitySystemComponent.IsValid() &&
		bCanEverBeTakenDown &&
		(HPRatio <= 0.25f) &&
		!AbilitySystemComponent->HasMatchingGameplayTag(BeingTakendownTag)
		)
	{
		return true;
	}
	
	return IGSDamageable::IsAvailableForTakedown_Implementation(TakedownComponent);
}

float ASWBaseCharacter::GetTakedownDuration_Implementation(UPrimitiveComponent* TakedownComponent) const
{
	return IGSDamageable::GetTakedownDuration_Implementation(TakedownComponent);
}

void ASWBaseCharacter::PreTakedown_Implementation(AActor* Takedowner, UPrimitiveComponent* TakedownComponent)
{
	const float HPRatio = GetHealth() / GetMaxHealth();
	if (AbilitySystemComponent.IsValid() && (HPRatio <= 0.25f) && HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(TakendownEffect, 1.f, AbilitySystemComponent->MakeEffectContext());
		if (EffectSpecHandle.IsValid())
		{
			FGameplayEffectSpec* EffectSpec = EffectSpecHandle.Data.Get();
			EffectSpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), GetMaxHealth());
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpec);
		}
	}
}

void ASWBaseCharacter::PostTakedown_Implementation(AActor* Takedowner, UPrimitiveComponent* TakedownComponent)
{
	const float HPRatio = GetHealth() / GetMaxHealth();
	if (AbilitySystemComponent.IsValid() && (HPRatio <= 0.25f) && HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(TakendownEffect, 1.f, AbilitySystemComponent->MakeEffectContext());
		if (EffectSpecHandle.IsValid())
		{
			FGameplayEffectSpec* EffectSpec = EffectSpecHandle.Data.Get();
			EffectSpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), GetMaxHealth());
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpec);
		}
	}
}

void ASWBaseCharacter::GetPreTakedownSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type,
	UPrimitiveComponent* TakedownComponent) const
{
	const float HPRatio = GetHealth() / GetMaxHealth();
	if (AbilitySystemComponent.IsValid() && (HPRatio <= 0.25f))
	{
		bShouldSync = true;
		Type = EAbilityTaskNetSyncType::OnlyClientWait;
		return;
	}
	
	IGSDamageable::GetPreTakedownSyncType_Implementation(bShouldSync, Type, TakedownComponent);
}

void ASWBaseCharacter::CancelTakedown_Implementation(UPrimitiveComponent* TakedownComponent)
{
	const float HPRatio = GetHealth() / GetMaxHealth();
	if (AbilitySystemComponent.IsValid() && (HPRatio <= 0.25f) && HasAuthority())
	{
		FGameplayTagContainer CancelTags(FGameplayTag::RequestGameplayTag("Ability.TakenDown"));
		AbilitySystemComponent->CancelAbilities(&CancelTags);
	}
}

FSimpleMulticastDelegate* ASWBaseCharacter::GetTargetCancelTakedownDelegate(UPrimitiveComponent* TakedownComponent)
{
	return &TakedownCanceledDelegate;
}

bool ASWBaseCharacter::IsStatusBarAvailable_Implementation() const
{
	if (UIFloatingStatusBar)
	{
		return true;
	}

	return false;
}

void ASWBaseCharacter::FadeInStatusBar_Implementation() const
{
	if (UIFloatingStatusBar)
	{
		UIFloatingStatusBar->PlayFadeIn();
	}
}

void ASWBaseCharacter::FadeOutStatusBar_Implementation() const
{
	if (UIFloatingStatusBar)
	{
		UIFloatingStatusBar->PlayFadeOut();
	}
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

float ASWBaseCharacter::GetMoveSpeed() const
{
	if (AttributeSet.IsValid())
	{
		return AttributeSet->GetMoveSpeed();
	}

	return 0.0f;
}

float ASWBaseCharacter::GetMoveSpeedBaseValue() const
{
	if (AttributeSet.IsValid())
	{
		return AttributeSet->GetMoveSpeedAttribute().GetGameplayAttributeData(AttributeSet.Get())->GetBaseValue();
	}

	return 0.0f;
}

void ASWBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
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
		UE_LOG(LogTemp, Error, TEXT("%s() Missing ASC for %s."), *FString(__FUNCTION__), *GetName());
		return;
	}

	if (!DefaultAttributes)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing DefaultAttributes for %s. Please fill in the character's Blueprint."), *FString(__FUNCTION__), *GetName());
		return;
	}

	// Can run on Server and Client
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributes, GetCharacterLevel(), EffectContext);
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


