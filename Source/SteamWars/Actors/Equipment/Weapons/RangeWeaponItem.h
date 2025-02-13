#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Actors/Equipment/EquipableItem.h"
#include "RangeWeaponItem.generated.h"

class UPaperSprite;
class USWAbilitySystemComponent;
struct FGameplayAbilitySpecHandle;
class USWGameplayAbility;
class ASWGATA_LineTrace;
class ASWGATA_LineTraceWithBloom;
class ASWGATA_SphereTrace;
enum class ESWAbilityInputID : uint8;
class ASWFPSCharacter;
class UWeaponBarrelComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponAmmoChangedDelegate, int32, OldValue, int32, NewValue);

UCLASS(Blueprintable)
class STEAMWARS_API ARangeWeaponItem : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ARangeWeaponItem();

	UPROPERTY(BlueprintReadWrite)
	bool bSpawnWithCollision;
	
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnMaxPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnSecondaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnMaxSecondaryClipAmmoChanged;
	
	UWeaponBarrelComponent* GetWeaponBarrelComponent() const { return WeaponBarrel; }
	float GetRateOfFire() const { return RateOfFire; }

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag WeaponTag;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTagContainer RestrictedPickupTags;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* PrimaryIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* SecondaryIcon;

	// UI HUD Primary Clip Icon when equipped
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* PrimaryClipIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* SecondaryClipIcon;

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag FireMode;
	FGameplayTag FullAutoFireMode;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag PrimaryAmmoType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag SecondaryAmmoType;

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	void SetOwningCharacter(ASWFPSCharacter* InOwningCharacter);

	// Pickup on touch
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	TSubclassOf<class USWHUDReticle> GetPrimaryHUDReticleClass() const;

	// Called when the player equips this weapon
	virtual void Equip();

	// Called when the player unequips this weapon
	virtual void UnEquip();

	virtual void AddAbilities();

	virtual void RemoveAbilities();

	virtual int32 GetAbilityLevel(ESWAbilityInputID AbilityID);

	// Resets things like fire mode to default
	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void ResetWeapon();

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetMaxPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetMaxSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetPrimaryClipAmmo(int32 NewPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetMaxPrimaryClipAmmo(int32 NewMaxPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetSecondaryClipAmmo(int32 NewSecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetMaxSecondaryClipAmmo(int32 NewMaxSecondaryClipAmmo);
	
	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual bool HasInfiniteAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Animation")
	UAnimMontage* GetEquipMontage() const;

	UFUNCTION(NetMulticast, Reliable)
	void OnDropped(FVector NewLocation);
	virtual void OnDropped_Implementation(FVector NewLocation);
	virtual bool OnDropped_Validate(FVector NewLocation);
	
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Audio")
	class USoundCue* GetPickupSound() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	FText GetDefaultStatusText() const;

	// Getter for LineTraceTargetActor. Spawns it if it doesn't exist yet.
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Targeting")
	ASWGATA_LineTrace* GetLineTraceTargetActor();

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Targeting")
	ASWGATA_LineTraceWithBloom* GetLineTraceWithBloomTargetActor();

	// Getter for SphereTraceTargetActor. Spawns it if it doesn't exist yet.
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Targeting")
	ASWGATA_SphereTrace* GetSphereTraceTargetActor();

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASShooter|GSWeapon")
	FText StatusText;
	
	FVector GetADSOffset() const;
	
protected:
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWeaponBarrelComponent* WeaponBarrel;

	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* CollisionComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parametrs", meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float RateOfFire = 600.0f;

	UPROPERTY()
	USWAbilitySystemComponent* AbilitySystemComponent;

	// How much ammo in the clip the gun starts with
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_PrimaryClipAmmo, Category = "GASShooter|GSWeapon|Ammo")
	int32 PrimaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxPrimaryClipAmmo, Category = "GASShooter|GSWeapon|Ammo")
	int32 MaxPrimaryClipAmmo;

	// How much ammo in the clip the gun starts with. Used for things like rifle grenades.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_SecondaryClipAmmo, Category = "GASShooter|GSWeapon|Ammo")
	int32 SecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxSecondaryClipAmmo, Category = "GASShooter|GSWeapon|Ammo")
	int32 MaxSecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Ammo")
	bool bInfiniteAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	TSubclassOf<class USWHUDReticle> PrimaryHUDReticleClass;
	/*UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	TSubclassOf<class UGSHUDReticle> PrimaryHUDReticleClass;*/

	UPROPERTY()
	ASWGATA_LineTrace* LineTraceTargetActor;

	UPROPERTY()
	ASWGATA_LineTraceWithBloom* LineTraceWithBloomTargetActor;

	UPROPERTY()
	ASWGATA_SphereTrace* SphereTraceTargetActor;

	UPROPERTY(VisibleAnywhere, Category = "GASShooter|GSWeapon")
	USkeletalMeshComponent* WeaponMesh1P;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon")
	TSubclassOf<UAnimInstance> WeaponAnimLinkLayer1P;

	// How much to offset the viewmodel to align to the center of the camera when adsing
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FVector ADSOffset;

	// Relative Location of weapon 3P Mesh when in pickup mode
	// 1P weapon mesh is invisible so it doesn't need one
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FVector WeaponMesh3PickupRelativeLocation;

	// Relative Location Offset of the offset root
	// This is only necessary because right now, I'm using first person animations from all over around the internet/marketplace
	// so they are not consistent in how the character hierarchy and viewport location are setup. This offset will fixes that for each weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	FVector OffsetRootLocationOffset;

	// Relative Location of weapon 1P Mesh when equipped
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	FVector WeaponMesh1PEquippedRelativeLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	FRotator WeaponMesh1PEquippedRelativeRotation = FRotator{0.f, 0.f, -90.f};

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GASShooter|GSWeapon")
	ASWFPSCharacter* OwningCharacter;

	UPROPERTY(EditAnywhere, Category = "GASShooter|GSWeapon")
	TArray<TSubclassOf<USWGameplayAbility>> Abilities;

	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|GSWeapon")
	FGameplayTag DefaultFireMode;

	// Things like fire mode for rifle
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon")
	FText DefaultStatusText;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|Animation")
	UAnimMontage* Equip1PMontage;

	// Sound played when player picks it up
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|Audio")
	class USoundCue* PickupSound;

	// Cache tags
	FGameplayTag WeaponPrimaryInstantAbilityTag;
	FGameplayTag WeaponSecondaryInstantAbilityTag;
	FGameplayTag WeaponAlternateInstantAbilityTag;
	FGameplayTag WeaponIsFiringTag;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	// Called when the player picks up this weapon
	virtual void PickUpOnTouch(ASWFPSCharacter* InCharacter);

	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_SecondaryClipAmmo(int32 OldSecondaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxSecondaryClipAmmo(int32 OldMaxSecondaryClipAmmo);


	/////////////////////////////////// Destiny-like Recoil
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Recoil")
	float SampleRecoilDirection(float x);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GASShooter|Recoil") // might want to make this a part of attribute set?
	float RecoilStat = 70.f;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Recoil")
	void StartRecoil();
	bool bIsRecoilActive;

	UPROPERTY(EditAnywhere)
	float BaseRecoilPitchForce = 8.f;
	float InitialRecoilPitchForce;
	float RecoilPitchDamping;
	float RecoilPitchVelocity;

	UPROPERTY(EditAnywhere)
	float BaseRecoilYawForce = 8.f;
	float InitialRecoilYawForce;
	float RecoilYawDamping;
	float RecoilYawVelocity;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Recoil")
	void StartRecoilRecovery();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASShooter|Recoil")
	float TargetingSpreadMax = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASShooter|Recoil")
	float TargetingSpreadMaxADS = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASShooter|Recoil")
	float TargetingSpreadIncrement = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASShooter|Recoil")
	float BaseSpread = 1.f;
	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|Recoil")
	float CurrentTargetingSpread = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASShooter|Recoil")
	float SpreadIncrementADSMod = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASShooter|Recoil")
	float SpreadRecoveryInterpSpeed = 20.f;
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASShooter|Recoil")
	//float SpreadRecoveryInterpSpeedAiming = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASShooter|Recoil")
	bool bIsUseADSStabilizer = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASShooter|Recoil", meta=(EditCondition="bIsUseADSStabilizer"))
	float MaxADSHeat = 10.f;
	float CurrentADSHeat = 0.f;

	// When the heat value reaches its peak, this value (0 - 100 percent) is the amount to reduce the recoil
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GASShooter|Recoil", meta = (EditCondition = "bIsUseADSStabilizer"))
	float ADSHeatModifierMax = 0.75;

public:
	bool bIsRecoilPitchRecoveryActive;
	bool bIsRecoilYawRecoveryActive;

	UPROPERTY(BlueprintReadWrite, Category = "GASShooter|Recoil")
	bool bIsRecoilNeutral = true;

	UPROPERTY(BlueprintReadWrite, Category = "GASShooter|Recoil")
	bool bUpdateRecoilPitchCheckpointInNextShot = false;

	UPROPERTY(BlueprintReadWrite, Category = "GASShooter|Recoil")
	bool bUpdateRecoilYawCheckpointInNextShot = false;

	UPROPERTY(BlueprintReadWrite, Category = "GASShooter|Recoil")
	FRotator RecoilCheckpoint;

	

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Recoil")
	void IncrementSpread();

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Recoil")
	float GetCurrentSpread() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Recoil")
	void ResetADSHeat();

private:
	FTimerHandle ShotTimerHandle;

	float GetShotTimerInterval();
};
