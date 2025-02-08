#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "../SWBaseCharacter.h"
#include "Characters/CharacterComponents/AbilitySystem/AttributeSet/AmmoAttributeSet.h"
#include "Player/SWPlayerState.h"
#include "SWFPSCharacter.generated.h"

struct FInputActionValue;
class ARangeWeaponItem;
enum class ESWAbilityInputID : uint8;

UENUM(BlueprintType)
enum class ESWHeroWeaponState : uint8
{
	// 0
	Rifle					UMETA(DisplayName = "Rifle"),
	// 1
	RifleAiming				UMETA(DisplayName = "Rifle Aiming"),
	// 2
	RocketLauncher			UMETA(DisplayName = "Rocket Launcher"),
	// 3
	RocketLauncherAiming	UMETA(DisplayName = "Rocket Launcher Aiming")
};

USTRUCT()
struct STEAMWARS_API FSWHeroInventory
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<ARangeWeaponItem*> Weapons;

	// Consumable items

	// Passive items like armor

	// Door keys

	// Etc
};

UCLASS()
class STEAMWARS_API ASWFPSCharacter : public ASWBaseCharacter
{
	GENERATED_BODY()

public:
	ASWFPSCharacter(const class FObjectInitializer& ObjectInitializer);

	/*==========================Tags==========================*/
	FGameplayTag CurrentWeaponTag;
	// Cache tags
	FGameplayTag NoWeaponTag;
	FGameplayTag WeaponChangingDelayReplicationTag;
	FGameplayTag WeaponAmmoTypeNoneTag;
	FGameplayTag WeaponAbilityTag;
	/*========================================================*/

	// GAS Server only
	virtual void PossessedBy(AController* NewController) override;
	
	class USWFloatingStatusBarWidget* GetFloatingStatusBar();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GAS|Character")
	USkeletalMeshComponent* GetFPSMesh() const { return FirstPersonMeshComponent; }
	
	float GetADSAlpha() const { return ADSAlpha; }

	virtual void FinishDying() override;

	/*==========================WeaponInventory==========================*/
	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	ARangeWeaponItem* GetCurrentWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	bool AddWeaponToInventory(ARangeWeaponItem* NewWeapon, bool bEquipWeapon = false);

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	bool RemoveWeaponFromInventory(ARangeWeaponItem* WeaponToRemove);

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	void RemoveAllWeaponsFromInventory();

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	void EquipWeapon(ARangeWeaponItem* NewWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(ARangeWeaponItem* NewWeapon);
	void ServerEquipWeapon_Implementation(ARangeWeaponItem* NewWeapon);
	bool ServerEquipWeapon_Validate(ARangeWeaponItem* NewWeapon);
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	virtual void NextWeapon();

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	virtual void PreviousWeapon();
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	int32 GetNumWeapons() const;
	/*====================================================================*/

	/*============================AmmoAttributes==========================*/
	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	int32 GetMaxPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	int32 GetPrimaryReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	int32 GetSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	int32 GetMaxSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	int32 GetSecondaryReserveAmmo() const;
	/*====================================================================*/

	FName GetWeaponAttachPoint();
protected:
	virtual void BeginPlay() override;

	/*-------------------Components-------------------*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	class USkeletalMeshComponent* FirstPersonMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	class UInputDataAsset* InputActions;
	/*------------------------------------------------*/

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS|Camera")
	float BaseTurnRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS|Camera")
	float BaseLookUpRate;

	UPROPERTY(BlueprintReadWrite, Category = "GAS|Character|Procedural FP Animation|ADS")
	float ADSAlpha{ 0.f };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS|Character")
	TSubclassOf<UGameplayEffect> DeathEffect;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSHeroCharacter")
	FName WeaponAttachPoint;

	void BindASCInput();
	bool bASCInputBound;
public:
	bool bChangedWeaponLocally;

	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	FSWHeroInventory Inventory;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS|Inventory")
	TArray<TSubclassOf<ARangeWeaponItem>> DefaultInventoryWeaponClasses;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	ARangeWeaponItem* CurrentWeapon;

	UPROPERTY()
	class UAmmoAttributeSet* AmmoAttributeSet;
	

	// Attribute changed delegate handles
	FDelegateHandle PrimaryReserveAmmoChangedDelegateHandle;
	FDelegateHandle SecondaryReserveAmmoChangedDelegateHandle;

	// Tag changed delegate handles
	FDelegateHandle WeaponChangingDelayReplicationTagChangedDelegateHandle;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintNativeEvent)
	void CharacterInitialSpawnDefaultInventory();

	UFUNCTION()
	void InitializeFloatingStatusBar();

	// Client only
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;

	UFUNCTION(BlueprintCallable)
	void SpawnDefaultInventory();

	bool DoesWeaponExistInInventory(ARangeWeaponItem* InWeapon);

	void SetCurrentWeapon(ARangeWeaponItem* NewWeapon, ARangeWeaponItem* LastWeapon);

	// Unequips the specified weapon. Used when OnRep_CurrentWeapon fires.
	void UnEquipWeapon(ARangeWeaponItem* WeaponToUnEquip);

	// Unequips the current weapon. Used if for example we drop the current weapon.
	void UnEquipCurrentWeapon();

	UFUNCTION()
	virtual void CurrentWeaponPrimaryClipAmmoChanged(int32 OldPrimaryClipAmmo, int32 NewPrimaryClipAmmo);

	UFUNCTION()
	virtual void CurrentWeaponSecondaryClipAmmoChanged(int32 OldSecondaryClipAmmo, int32 NewSecondaryClipAmmo);

	// Attribute changed callbacks
	virtual void CurrentWeaponPrimaryReserveAmmoChanged(const FOnAttributeChangeData& Data);
	virtual void CurrentWeaponSecondaryReserveAmmoChanged(const FOnAttributeChangeData& Data);

	// Tag changed callbacks
	virtual void WeaponChangingDelayReplicationTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION()
	void OnRep_CurrentWeapon(ARangeWeaponItem* LastWeapon);

	UFUNCTION()
	void OnRep_Inventory();

	void OnAbilityActivationFailed(const UGameplayAbility* FailedAbility, const FGameplayTagContainer& FailTags);
	
	// The CurrentWeapon is only automatically replicated to simulated clients.
	// The autonomous client can use this to request the proper CurrentWeapon from the server when it knows it may be
	// out of sync with it from predictive client-side changes.
	UFUNCTION(Server, Reliable)
	void ServerSyncCurrentWeapon();
	void ServerSyncCurrentWeapon_Implementation();
	bool ServerSyncCurrentWeapon_Validate();
	
	// The CurrentWeapon is only automatically replicated to simulated clients.
	// Use this function to manually sync the autonomous client's CurrentWeapon when we're ready to.
	// This allows us to predict weapon changes (changing weapons fast multiple times in a row so that the server doesn't
	// replicate and clobber our CurrentWeapon).
	UFUNCTION(Client, Reliable)
	void ClientSyncCurrentWeapon(ARangeWeaponItem* InWeapon);
	void ClientSyncCurrentWeapon_Implementation(ARangeWeaponItem* InWeapon);
	bool ClientSyncCurrentWeapon_Validate(ARangeWeaponItem* InWeapon);
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/*---------------Movement---------------*/
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartCrouch();
	void StopCrouch();
	virtual void Jump() override;
	virtual void StopJumping() override;
	/*---------------------------------------*/
	/*---------------Abilities---------------*/
	void InvokeAbility(const FInputActionValue& Value, ESWAbilityInputID Id, bool IsActive);
	/*---------------------------------------*/
	
	void InitAbilitySystemComponent();

	void SetLocalInputToASC(bool bIsPressed, const ESWAbilityInputID AbilityInputID);
};
