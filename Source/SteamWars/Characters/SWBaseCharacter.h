#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "SWTypes.h"
#include "SWBaseCharacter.generated.h"

class USWGameplayAbility;
class USWCharacterEquipmentComponent;
class UGameplayAbility;
class UGameplayEffect;
class USWAttributeSet;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterBaseHitReactDelegate, EGDHitReactDirection, Direction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterDiedDelegate, ASWBaseCharacter*, Character);

UCLASS()
class STEAMWARS_API ASWBaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASWBaseCharacter();

	UPROPERTY(BlueprintAssignable, Category = "Character | HitReact")
	FCharacterBaseHitReactDelegate ShowHitReact;

	UPROPERTY(BlueprintAssignable, Category = "Character | Died")
	FCharacterDiedDelegate OnCharacterDied;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual USWAttributeSet* GetAttributeSet() const;

	UFUNCTION(BlueprintCallable, Category = "GAS | Character")
	virtual int32 GetAbilityLevel(ESWAbilityInputID AbilityID) const;
	
	virtual void RemoveCharacterAbilities();
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Character")
	virtual bool IsAlive() const;
	
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "GAS|Character")
	virtual void FinishDying();

	UFUNCTION(BlueprintCallable)
	EGDHitReactDirection GetHitReactDirection(const FVector& ImpactPoint);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	virtual void PlayHitReact(FGameplayTag HitDirection, AActor* DamageCauser);
	virtual void PlayHitReact_Implementation(FGameplayTag HitDirection, AActor* DamageCauser);
	virtual bool PlayHitReact_Validate(FGameplayTag HitDirection, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "GASDocumentation|GDCharacter|Attributes")
	int32 GetCharacterLevel() const;
	
	const USWCharacterEquipmentComponent* GetEquipmentComponent() const;

	/*---------------------------------Attributes------------------------------------*/
	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	float GetMaxHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	float GetMaxStamina() const;	
	/*-------------------------------------------------------------------------------*/

protected:
	UPROPERTY()
	TWeakObjectPtr<class USWAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TWeakObjectPtr<USWAttributeSet> AttributeSet;
	
	FGameplayTag HitDirectionFrontTag;
	FGameplayTag HitDirectionBackTag;
	FGameplayTag HitDirectionRightTag;
	FGameplayTag HitDirectionLeftTag;
	FGameplayTag DeadTag;
	FGameplayTag EffectRemoveOnDeathTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS|Animation")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<TSubclassOf<USWGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASDocumentation|Abilities")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;
	
	void GiveDefaultAbilities();
	void InitDefaultAttributes() const;
	virtual void AddStartupEffects();
	
	virtual void SetHealth(float Health);
	virtual void SetStamina(float Stamina);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	USWCharacterEquipmentComponent* CharacterEquipmentComponent;
	
	UPROPERTY()
	uint8 bAbilitiesInitialized:1;
};
