#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "SWTypes.h"
#include "CharacterComponents/AbilitySystem/SWDamageable.h"
#include "SWBaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterBaseHitReactDelegate, ESWHitReactDirection, Direction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterDiedDelegate, ASWBaseCharacter*, Character);

UCLASS()
class STEAMWARS_API ASWBaseCharacter : public ACharacter, public IAbilitySystemInterface, public IGSDamageable
{
	GENERATED_BODY()

public:
	ASWBaseCharacter(const class FObjectInitializer& ObjectInitializer);

	/*===================Delegates===================*/
	UPROPERTY(BlueprintAssignable, Category = "GAS | Character | HitReact")
	FCharacterBaseHitReactDelegate ShowHitReact;

	UPROPERTY(BlueprintAssignable, Category = "GAS | Character | Died")
	FCharacterDiedDelegate OnCharacterDied;
	/*==============================================*/
	
	/*===================GAS===================*/
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual class USWAttributeSet* GetAttributeSet() const;

	UFUNCTION(BlueprintCallable, Category = "GAS | Character")
	virtual int32 GetAbilityLevel(ESWAbilityInputID AbilityID) const;
	
	virtual void RemoveCharacterAbilities();
	/*=========================================*/

	/*===================Die===================*/
	UFUNCTION(BlueprintCallable, Category = "GAS | Character")
	virtual bool IsAlive() const;
	
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "GAS | Character")
	virtual void FinishDying();
	/*=========================================*/

	/*==========================Attributes==========================*/
	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	float GetMaxHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	float GetMoveSpeed() const;
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	float GetMoveSpeedBaseValue() const;
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Character|Attributes")
	int32 GetCharacterLevel() const;
	/*===============================================================*/

	/*==========================HitReact==========================*/
	UFUNCTION(BlueprintCallable)
	ESWHitReactDirection GetHitReactDirection(const FVector& ImpactPoint);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	virtual void PlayHitReact(FGameplayTag HitDirection, AActor* DamageCauser);
	virtual void PlayHitReact_Implementation(FGameplayTag HitDirection, AActor* DamageCauser);
	virtual bool PlayHitReact_Validate(FGameplayTag HitDirection, AActor* DamageCauser);
	/*===============================================================*/

	/**
	* Damageable interface
	*/

	/**
	* This Character can be taken down by other heroes when:
	* HP is less than 25% - to knock them down
	*/
	virtual bool IsAvailableForTakedown_Implementation(UPrimitiveComponent* TakedownComponent) const override;

	/**
	* How long to takedown with this player:
	* should be instant
	*/
	virtual float GetTakedownDuration_Implementation(UPrimitiveComponent* TakedownComponent) const override;

	/**
	* takedown:
	* activate takedown GA (plays animation)
	*/
	virtual void PreTakedown_Implementation(AActor* Takedowner, UPrimitiveComponent* TakedownComponent) override;

	/**
	* takedown:
	* apply takedown GE
	*/
	virtual void PostTakedown_Implementation(AActor* Takedowner, UPrimitiveComponent* TakedownComponent) override;

	virtual void GetPreTakedownSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* TakedownComponent) const override;

	/**
	* Cancel takedown:
	* takedown - cancel takedown ability
	*/
	virtual void CancelTakedown_Implementation(UPrimitiveComponent* TakedownComponent) override;

	/**
	* Get the delegate for this Actor canceling takedown:
	* cancel being taken down if killed
	*/
	FSimpleMulticastDelegate* GetTargetCancelTakedownDelegate(UPrimitiveComponent* TakedownComponent) override;
	FSimpleMulticastDelegate TakedownCanceledDelegate;
	
	/*==========================StatusBar==========================*/
	virtual bool IsStatusBarAvailable_Implementation() const override;
	virtual void FadeInStatusBar_Implementation() const override;
	virtual void FadeOutStatusBar_Implementation() const override;
	/*===============================================================*/
protected:
	/*===================Tags===================*/
	FGameplayTag HitDirectionFrontTag;
	FGameplayTag HitDirectionBackTag;
	FGameplayTag HitDirectionRightTag;
	FGameplayTag HitDirectionLeftTag;
	FGameplayTag DeadTag;
	FGameplayTag EffectRemoveOnDeathTag;
	FGameplayTag BeingTakendownTag;
	/*=========================================*/
	
	/*==========================GAS==========================*/
	// Reference to the ASC. It will live on the PlayerState or here if the character doesn't have a PlayerState.
	UPROPERTY()
	TWeakObjectPtr<class USWAbilitySystemComponent> AbilitySystemComponent;
	// Reference to the AttributeSetBase. It will live on the PlayerState or here if the character doesn't have a PlayerState.
	UPROPERTY()
	TWeakObjectPtr<USWAttributeSet> AttributeSet;
	/*========================================================*/

	/*==========================Sturtup=========================*/
	UPROPERTY(EditDefaultsOnly, Category = "GAS | Character | Abilities")
	TArray<TSubclassOf<class USWGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS | Character | Abilities")
	TSubclassOf<class UGameplayEffect> DefaultAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS | Character | Abilities")
	TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS | Character | Abilities")
	TSubclassOf<class UGameplayEffect> TakendownEffect;
	/*========================================================*/
	
	/*==========================UI==========================*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS | Character | UI")
	TSubclassOf<class USWFloatingStatusBarWidget> UIFloatingStatusBarClass;

	UPROPERTY()
	class USWFloatingStatusBarWidget* UIFloatingStatusBar;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GASShooter|UI")
	class UWidgetComponent* UIFloatingStatusBarComponent;
	/*========================================================*/

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS | Character | Name")
	FText CharacterName;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS | Character | Animation")
	UAnimMontage* DeathMontage;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS | Character|Takedown")
	bool bCanEverBeTakenDown;

	virtual void BeginPlay() override;
	
	void GiveDefaultAbilities();
	void InitDefaultAttributes() const;
	virtual void AddStartupEffects();
	
	virtual void SetHealth(float Health);
	virtual void SetStamina(float Stamina);
	
	
};
