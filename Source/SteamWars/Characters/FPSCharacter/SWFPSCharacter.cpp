#include "SWFPSCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SWTypes.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../InputData/InputDataAsset.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Characters/CharacterComponents/SWCharacterEquipmentComponent.h"
#include "Characters/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"
#include "Characters/CharacterComponents/AbilitySystem/SWBlueprintFunctionLibrary.h"
#include "Characters/CharacterComponents/AbilitySystem/AttributeSet/AmmoAttributeSet.h"
#include "Characters/CharacterComponents/AbilitySystem/AttributeSet/SWAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "GameMode/SWGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/SWPlayerController.h"
#include "Player/SWPlayerState.h"
#include "UI/SWFloatingStatusBarWidget.h"


ASWFPSCharacter::ASWFPSCharacter(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;
	bASCInputBound = false;
	bChangedWeaponLocally = false;
	NoWeaponTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.None"));
	WeaponChangingDelayReplicationTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.IsChangingDelayReplication"));
	WeaponAmmoTypeNoneTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));
	WeaponAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon"));
	CurrentWeaponTag = NoWeaponTag;
	Inventory = FSWHeroInventory();

	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;

	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	FirstPersonMeshComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -86.f));
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bCastDynamicShadow = false;
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, SocketFPCamera);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	if(UIFloatingStatusBarComponent)
	{
		UIFloatingStatusBarComponent = CreateDefaultSubobject<UWidgetComponent>(FName("UIFloatingStatusBarComponent"));
		UIFloatingStatusBarComponent->SetupAttachment(RootComponent);
		UIFloatingStatusBarComponent->SetRelativeLocation(FVector(0, 0, 120));
		UIFloatingStatusBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
		UIFloatingStatusBarComponent->SetDrawSize(FVector2D(500, 500));
	}
	
	/*UIFloatingStatusBarClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/GASShooter/UI/UI_FloatingStatusBar_Hero.UI_FloatingStatusBar_Hero_C"));
	if (!UIFloatingStatusBarClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Failed to find UIFloatingStatusBarClass. If it was moved, please update the reference location in C++."), *FString(__FUNCTION__));
	}*/
	
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;

	GetCharacterMovement()->bOrientRotationToMovement = 0;
}

void ASWFPSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ASWPlayerState* PS = GetPlayerState<ASWPlayerState>();
	if (PS)
	{
		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		AbilitySystemComponent = Cast<USWAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// AI won't have PlayerControllers so we can init again here just to be sure. No harm in initing twice for heroes that have PlayerControllers.
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		WeaponChangingDelayReplicationTagChangedDelegateHandle = AbilitySystemComponent->RegisterGameplayTagEvent(WeaponChangingDelayReplicationTag)
			.AddUObject(this, &ASWFPSCharacter::WeaponChangingDelayReplicationTagChanged);

		// Set the AttributeSetBase for convenience attribute functions
		AttributeSet = PS->GetAttributeSet();

		AmmoAttributeSet = PS->GetAmmoAttributeSet();

		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that possession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.
		InitDefaultAttributes();

		AddStartupEffects();

		GiveDefaultAbilities();

		ASWPlayerController* PC = Cast<ASWPlayerController>(GetController());
		if (PC)
		{
			PC->CreateHUD();
		}

		if (AbilitySystemComponent->GetTagCount(DeadTag) > 0)
		{
			// Set Health/Mana/Stamina to their max. This is only necessary for *Respawn*.
			SetHealth(GetMaxHealth());
			SetStamina(GetMaxStamina());
		}

		// Remove Dead tag
		AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(DeadTag));

		InitializeFloatingStatusBar();

		// If player is host on listen server, the floating status bar would have been created for them from BeginPlay before player possession, hide it
		if (IsLocallyControlled() && IsPlayerControlled() && UIFloatingStatusBarComponent && UIFloatingStatusBar)
		{
			UIFloatingStatusBarComponent->SetVisibility(false, true);
		}
	}
}

class USWFloatingStatusBarWidget* ASWFPSCharacter::GetFloatingStatusBar()
{
	return UIFloatingStatusBar;
}

void ASWFPSCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Pause();

	IsEndPlay = true;
	
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = NoWeaponTag;
		AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
	}
	Super::EndPlay(EndPlayReason);
}

void ASWFPSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CharacterInitialSpawnDefaultInventory();
}

void ASWFPSCharacter::CharacterInitialSpawnDefaultInventory_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Called from %s with original CharacterInitialSpawnDefaultInventory implementation."), *GetName());
	UE_LOG(LogTemp, Warning, TEXT("%s() original implementation called. Override this in BP instead!."), *FString(__FUNCTION__));
}

void ASWFPSCharacter::InitializeFloatingStatusBar()
{
	// Only create once
	if (UIFloatingStatusBar || !AbilitySystemComponent.IsValid())
	{
		return;
	}

	// Don't create for locally controlled player. We could add a game setting to toggle this later.
	if (IsPlayerControlled() && IsLocallyControlled())
	{
		return;
	}

	// Need a valid PlayerState
	if (!GetPlayerState())
	{
		return;
	}

	// Setup UI for Locally Owned Players only, not AI or the server's copy of the PlayerControllers
	ASWPlayerController* PC = Cast<ASWPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC && PC->IsLocalPlayerController())
	{
		if (UIFloatingStatusBarClass)
		{
			UIFloatingStatusBar = CreateWidget<USWFloatingStatusBarWidget>(PC, UIFloatingStatusBarClass);
			if (UIFloatingStatusBar && UIFloatingStatusBarComponent)
			{
				UIFloatingStatusBarComponent->SetWidget(UIFloatingStatusBar);

				// Setup the floating status bar
				UIFloatingStatusBar->SetHealthPercentage(GetHealth() / GetMaxHealth());
				UIFloatingStatusBar->OwningCharacter = this;
				UIFloatingStatusBar->SetCharacterName(CharacterName);
			}
		}
	}
}

void ASWFPSCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
}

void ASWFPSCharacter::SpawnDefaultInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	int32 NumWeaponClasses = DefaultInventoryWeaponClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (!DefaultInventoryWeaponClasses[i])
		{
			// An empty item was added to the Array in blueprint
			continue;
		}

		ARangeWeaponItem* NewWeapon = GetWorld()->SpawnActorDeferred<ARangeWeaponItem>(DefaultInventoryWeaponClasses[i],
			FTransform::Identity, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		NewWeapon->bSpawnWithCollision = false;
		NewWeapon->FinishSpawning(FTransform::Identity);

		bool bEquipFirstWeapon = i == 0;
		UE_LOG(LogTemp, Warning, TEXT("bEquipFirstWeapon: %s"), (bEquipFirstWeapon ? TEXT("true") : TEXT("false")));
		AddWeaponToInventory(NewWeapon, bEquipFirstWeapon);
	}
}

bool ASWFPSCharacter::DoesWeaponExistInInventory(ARangeWeaponItem* InWeapon)
{
	for (ARangeWeaponItem* Weapon : Inventory.Weapons)
	{
		if (Weapon && InWeapon && Weapon->GetClass() == InWeapon->GetClass())
		{
			return true;
		}
	}

	return false;
}

void ASWFPSCharacter::SetCurrentWeapon(ARangeWeaponItem* NewWeapon, ARangeWeaponItem* LastWeapon)
{
	if (NewWeapon == LastWeapon)
	{
		return;
	}

	// Cancel active weapon abilities
	if (AbilitySystemComponent.IsValid())
	{
		FGameplayTagContainer AbilityTagsToCancel = FGameplayTagContainer(WeaponAbilityTag);
		AbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel);
	}

	UnEquipWeapon(LastWeapon);

	if (NewWeapon)
	{
		if (AbilitySystemComponent.IsValid())
		{
			// Clear out potential NoWeaponTag
			AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		}

		// Weapons coming from OnRep_CurrentWeapon won't have the owner set
		CurrentWeapon = NewWeapon;
		CurrentWeapon->Equip();
		CurrentWeapon->SetOwningCharacter(this);
		CurrentWeaponTag = CurrentWeapon->WeaponTag;

		if (AbilitySystemComponent.IsValid())
		{
			AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
		}

		ASWPlayerController* PC = GetController<ASWPlayerController>();
		if (PC && PC->IsLocalController())
		{
			PC->SetEquippedWeaponPrimaryIconFromSprite(CurrentWeapon->PrimaryIcon);
			PC->SetEquippedWeaponStatusText(CurrentWeapon->StatusText);
			PC->SetPrimaryClipAmmo(CurrentWeapon->GetPrimaryClipAmmo());
			PC->SetPrimaryReserveAmmo(GetPrimaryReserveAmmo());
			PC->SetHUDReticle(CurrentWeapon->GetPrimaryHUDReticleClass());
		}

		NewWeapon->OnPrimaryClipAmmoChanged.AddDynamic(this, &ASWFPSCharacter::CurrentWeaponPrimaryClipAmmoChanged);
		NewWeapon->OnSecondaryClipAmmoChanged.AddDynamic(this, &ASWFPSCharacter::CurrentWeaponSecondaryClipAmmoChanged);
		
		if (AbilitySystemComponent.IsValid())
		{
			PrimaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType)).AddUObject(this, &ASWFPSCharacter::CurrentWeaponPrimaryReserveAmmoChanged);
			SecondaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->SecondaryAmmoType)).AddUObject(this, &ASWFPSCharacter::CurrentWeaponSecondaryReserveAmmoChanged);
		}

		/*UAnimMontage* Equip1PMontage = CurrentWeapon->GetEquip1PMontage();
		if (Equip1PMontage && GetFirstPersonMesh())
		{
			GetFirstPersonMesh()->GetAnimInstance()->Montage_Play(Equip1PMontage);
		}*/

		/*UAnimMontage* Equip3PMontage = CurrentWeapon->GetEquip3PMontage();
		if (Equip3PMontage && GetThirdPersonMesh())
		{
			GetThirdPersonMesh()->GetAnimInstance()->Montage_Play(Equip3PMontage);
		}*/
	}
	else
	{
		// This will clear HUD, tags etc
		UnEquipCurrentWeapon();
	}
}

void ASWFPSCharacter::UnEquipWeapon(ARangeWeaponItem* WeaponToUnEquip)
{
	if (WeaponToUnEquip)
	{
		WeaponToUnEquip->OnPrimaryClipAmmoChanged.RemoveDynamic(this, &ASWFPSCharacter::CurrentWeaponPrimaryClipAmmoChanged);
		WeaponToUnEquip->OnSecondaryClipAmmoChanged.RemoveDynamic(this, &ASWFPSCharacter::CurrentWeaponSecondaryClipAmmoChanged);

		if (AbilitySystemComponent.IsValid())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAmmoAttributeSet::GetReserveAmmoAttributeFromTag(WeaponToUnEquip->PrimaryAmmoType)).Remove(PrimaryReserveAmmoChangedDelegateHandle);
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAmmoAttributeSet::GetReserveAmmoAttributeFromTag(WeaponToUnEquip->SecondaryAmmoType)).Remove(SecondaryReserveAmmoChangedDelegateHandle);
		}
		
		WeaponToUnEquip->UnEquip();
	}
}

void ASWFPSCharacter::UnEquipCurrentWeapon()
{
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = NoWeaponTag;
		AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
	}

	UnEquipWeapon(CurrentWeapon);
	CurrentWeapon = nullptr;

	ASWPlayerController* PC = GetController<ASWPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetEquippedWeaponPrimaryIconFromSprite(nullptr);
		PC->SetEquippedWeaponStatusText(FText());
		PC->SetPrimaryClipAmmo(0);
		PC->SetPrimaryReserveAmmo(0);
		PC->SetHUDReticle(nullptr);
	}
}

void ASWFPSCharacter::CurrentWeaponPrimaryClipAmmoChanged(int32 OldPrimaryClipAmmo, int32 NewPrimaryClipAmmo)
{
	ASWPlayerController* PC = GetController<ASWPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetPrimaryClipAmmo(NewPrimaryClipAmmo);
	}
}

void ASWFPSCharacter::CurrentWeaponSecondaryClipAmmoChanged(int32 OldSecondaryClipAmmo, int32 NewSecondaryClipAmmo)
{
	ASWPlayerController* PC = GetController<ASWPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetSecondaryClipAmmo(NewSecondaryClipAmmo);
	}
}

void ASWFPSCharacter::CurrentWeaponPrimaryReserveAmmoChanged(const FOnAttributeChangeData& Data)
{
	ASWPlayerController* PC = GetController<ASWPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetPrimaryReserveAmmo(Data.NewValue);
	}
}

void ASWFPSCharacter::CurrentWeaponSecondaryReserveAmmoChanged(const FOnAttributeChangeData& Data)
{
	ASWPlayerController* PC = GetController<ASWPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetSecondaryReserveAmmo(Data.NewValue);
	}
}

void ASWFPSCharacter::WeaponChangingDelayReplicationTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (CallbackTag == WeaponChangingDelayReplicationTag)
	{
		if (NewCount < 1)
		{
			// We only replicate the current weapon to simulated proxies so manually sync it when the weapon changing delay replication
			// tag is removed. We keep the weapon changing tag on for ~1s after the equip montage to allow for activating changing weapon
			// again without the server trying to clobber the next locally predicted weapon.
			ClientSyncCurrentWeapon(CurrentWeapon);
		}
	}
}

void ASWFPSCharacter::OnRep_CurrentWeapon(ARangeWeaponItem* LastWeapon)
{
	bChangedWeaponLocally = false;
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void ASWFPSCharacter::OnRep_Inventory()
{
	if (GetLocalRole() == ROLE_AutonomousProxy && Inventory.Weapons.Num() > 0 && !CurrentWeapon)
	{
		// Since we don't replicate the CurrentWeapon to the owning client, this is a way to ask the Server to sync
		// the CurrentWeapon after it's been spawned via replication from the Server.
		// The weapon spawning is replicated but the variable CurrentWeapon is not on the owning client.
		ServerSyncCurrentWeapon();
	}
}

void ASWFPSCharacter::OnAbilityActivationFailed(const UGameplayAbility* FailedAbility,
	const FGameplayTagContainer& FailTags)
{
	if (FailedAbility && FailedAbility->AbilityTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.IsChanging"))))
	{
		if (bChangedWeaponLocally)
		{
			// Ask the Server to resync the CurrentWeapon that we predictively changed
			UE_LOG(LogTemp, Warning, TEXT("%s Weapon Changing ability activation failed. Syncing CurrentWeapon. %s. %s"), *FString(__FUNCTION__),
				*USWBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()), *FailTags.ToString());

			ServerSyncCurrentWeapon();
		}
	}
}

void ASWFPSCharacter::ServerSyncCurrentWeapon_Implementation()
{
	ClientSyncCurrentWeapon(CurrentWeapon);
}

bool ASWFPSCharacter::ServerSyncCurrentWeapon_Validate()
{
	return true;
}

void ASWFPSCharacter::ClientSyncCurrentWeapon_Implementation(ARangeWeaponItem* InWeapon)
{
	ARangeWeaponItem* LastWeapon = CurrentWeapon;
	CurrentWeapon = InWeapon;
	OnRep_CurrentWeapon(LastWeapon);
}

bool ASWFPSCharacter::ClientSyncCurrentWeapon_Validate(ARangeWeaponItem* InWeapon)
{
	return true;
}

void ASWFPSCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ASWPlayerState* PS = GetPlayerState<ASWPlayerState>();
	if (PS)
	{
		// Set the ASC for clients. Server does this in PossessedBy.
		AbilitySystemComponent = Cast<USWAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor.
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);

		// Bind player input to the AbilitySystemComponent. Also called in SetupPlayerInputComponent because of a potential race condition.
		BindASCInput();

		AbilitySystemComponent->AbilityFailedCallbacks.AddUObject(this, &ASWFPSCharacter::OnAbilityActivationFailed);

		// Set the AttributeSetBase for convenience attribute functions
		AttributeSet = PS->GetAttributeSet();
		
		AmmoAttributeSet = PS->GetAmmoAttributeSet();

		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that posession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.
		InitDefaultAttributes();

		ASWPlayerController* PC = Cast<ASWPlayerController>(GetController());
		if (PC)
		{
			PC->CreateHUD();
		}
		
		if (CurrentWeapon)
		{
			// If current weapon repped before PlayerState, set tag on ASC
			AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
			// Update owning character and ASC just in case it repped before PlayerState
			CurrentWeapon->SetOwningCharacter(this);

			if (!PrimaryReserveAmmoChangedDelegateHandle.IsValid())
			{
				PrimaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType)).AddUObject(this, &ASWFPSCharacter::CurrentWeaponPrimaryReserveAmmoChanged);
			}
			if (!SecondaryReserveAmmoChangedDelegateHandle.IsValid())
			{
				SecondaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->SecondaryAmmoType)).AddUObject(this, &ASWFPSCharacter::CurrentWeaponSecondaryReserveAmmoChanged);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("!CurrentWeapon"));
		}

		if (AbilitySystemComponent->GetTagCount(DeadTag) > 0)
		{
			// Set Health/Mana/Stamina/Shield to their max. This is only for *Respawn*. It will be set (replicated) by the
			// Server, but we call it here just to be a little more responsive.
			SetHealth(GetMaxHealth());
			SetStamina(GetMaxStamina());
		}

		// Simulated on proxies don't have their PlayerStates yet when BeginPlay is called so we call it again here
		InitializeFloatingStatusBar();
	}
}

void ASWFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASWFPSCharacter, Inventory);
	DOREPLIFETIME_CONDITION(ASWFPSCharacter, CurrentWeapon, COND_SimulatedOnly);
}

void ASWFPSCharacter::FinishDying()
{
	if (!HasAuthority())
	{
		return;
	}

	ASWGameMode* GM = Cast<ASWGameMode>(GetWorld()->GetAuthGameMode());

	//TODO: hero died
	/*if (GM)
	{
		GM->HeroDied(GetController());
	}*/

	RemoveCharacterAbilities();

	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;
		EffectTagsToRemove.AddTag(EffectRemoveOnDeathTag);
		int32 NumEffectsRemoved = AbilitySystemComponent->RemoveActiveEffectsWithTags(EffectTagsToRemove);

		AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(DeathEffect->GetDefaultObject()), 1.0f, AbilitySystemComponent->MakeEffectContext());
	}

	OnCharacterDied.Broadcast(this);
	
	Super::FinishDying();
}

ARangeWeaponItem* ASWFPSCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

bool ASWFPSCharacter::AddWeaponToInventory(ARangeWeaponItem* NewWeapon, bool bEquipWeapon)
{
	if (DoesWeaponExistInInventory(NewWeapon))
	{
		UE_LOG(LogTemp, Warning, TEXT("DoesWeaponExistInInventory(NewWeapon)"));
		USoundCue* PickupSound = NewWeapon->GetPickupSound();

		/*if (PickupSound && IsLocallyControlled())
		{
			UGameplayStatics::SpawnSoundAttached(PickupSound, GetRootComponent());
		}*/

		if (GetLocalRole() < ROLE_Authority)
		{
			return false;
		}

		// Create a dynamic instant Gameplay Effect to give the primary and secondary ammo
		UGameplayEffect* GEAmmo = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("Ammo")));
		GEAmmo->DurationPolicy = EGameplayEffectDurationType::Instant;

		if (NewWeapon->PrimaryAmmoType != WeaponAmmoTypeNoneTag)
		{
			int32 Idx = GEAmmo->Modifiers.Num();
			GEAmmo->Modifiers.SetNum(Idx + 1);

			FGameplayModifierInfo& InfoPrimaryAmmo = GEAmmo->Modifiers[Idx];
			InfoPrimaryAmmo.ModifierMagnitude = FScalableFloat(NewWeapon->GetPrimaryClipAmmo());
			InfoPrimaryAmmo.ModifierOp = EGameplayModOp::Additive;
			InfoPrimaryAmmo.Attribute = UAmmoAttributeSet::GetReserveAmmoAttributeFromTag(NewWeapon->PrimaryAmmoType);
		}

		if (NewWeapon->SecondaryAmmoType != WeaponAmmoTypeNoneTag)
		{
			int32 Idx = GEAmmo->Modifiers.Num();
			GEAmmo->Modifiers.SetNum(Idx + 1);

			FGameplayModifierInfo& InfoSecondaryAmmo = GEAmmo->Modifiers[Idx];
			InfoSecondaryAmmo.ModifierMagnitude = FScalableFloat(NewWeapon->GetSecondaryClipAmmo());
			InfoSecondaryAmmo.ModifierOp = EGameplayModOp::Additive;
			InfoSecondaryAmmo.Attribute = UAmmoAttributeSet::GetReserveAmmoAttributeFromTag(NewWeapon->SecondaryAmmoType);
		}

		if (GEAmmo->Modifiers.Num() > 0)
		{
			AbilitySystemComponent->ApplyGameplayEffectToSelf(GEAmmo, 1.0f, AbilitySystemComponent->MakeEffectContext());
		}

		NewWeapon->Destroy();

		return false;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("!DoesWeaponExistInInventory(NewWeapon)"));
	}

	if (GetLocalRole() < ROLE_Authority)
	{
		return false;
	}

	Inventory.Weapons.Add(NewWeapon);
	NewWeapon->SetOwningCharacter(this);
	NewWeapon->AddAbilities();

	if (bEquipWeapon)
	{
		EquipWeapon(NewWeapon);
		ClientSyncCurrentWeapon(CurrentWeapon);
	}

	return true;
}

bool ASWFPSCharacter::RemoveWeaponFromInventory(ARangeWeaponItem* WeaponToRemove)
{
	if (DoesWeaponExistInInventory(WeaponToRemove))
	{
		if (WeaponToRemove == CurrentWeapon)
		{
			UnEquipCurrentWeapon();
		}

		Inventory.Weapons.Remove(WeaponToRemove);
		WeaponToRemove->RemoveAbilities();
		WeaponToRemove->SetOwningCharacter(nullptr);
		WeaponToRemove->ResetWeapon();

		// Add parameter to drop weapon?

		return true;
	}

	return false;
}

void ASWFPSCharacter::RemoveAllWeaponsFromInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	UnEquipCurrentWeapon();

	float radius = 50.0f;
	float NumWeapons = Inventory.Weapons.Num();

	for (int32 i = Inventory.Weapons.Num() - 1; i >= 0; i--)
	{
		ARangeWeaponItem* Weapon = Inventory.Weapons[i];
		RemoveWeaponFromInventory(Weapon);

		// Set the weapon up as a pickup

		float OffsetX = radius * FMath::Cos((i / NumWeapons) * 2.0f * PI);
		float OffsetY = radius * FMath::Sin((i / NumWeapons) * 2.0f * PI);
		Weapon->OnDropped(GetActorLocation() + FVector(OffsetX, OffsetY, 0.0f));
	}
}

void ASWFPSCharacter::EquipWeapon(ARangeWeaponItem* NewWeapon)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerEquipWeapon(NewWeapon);
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
		bChangedWeaponLocally = true;
	}
	else
	{
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
	}
}

void ASWFPSCharacter::ServerEquipWeapon_Implementation(ARangeWeaponItem* NewWeapon)
{
	EquipWeapon(NewWeapon);
}

bool ASWFPSCharacter::ServerEquipWeapon_Validate(ARangeWeaponItem* NewWeapon)
{
	return true;
}

void ASWFPSCharacter::NextWeapon()
{
	if (Inventory.Weapons.Num() < 2)
	{
		return;
	}

	int32 CurrentWeaponIndex = Inventory.Weapons.Find(CurrentWeapon);
	UnEquipCurrentWeapon();

	if (CurrentWeaponIndex == INDEX_NONE)
	{
		EquipWeapon(Inventory.Weapons[0]);
	}
	else
	{
		EquipWeapon(Inventory.Weapons[(CurrentWeaponIndex + 1) % Inventory.Weapons.Num()]);
	}
}

void ASWFPSCharacter::PreviousWeapon()
{
	if (Inventory.Weapons.Num() < 2)
	{
		return;
	}

	int32 CurrentWeaponIndex = Inventory.Weapons.Find(CurrentWeapon);

	UnEquipCurrentWeapon();

	if (CurrentWeaponIndex == INDEX_NONE)
	{
		EquipWeapon(Inventory.Weapons[0]);
	}
	else
	{
		int32 IndexOfPrevWeapon = FMath::Abs(CurrentWeaponIndex - 1 + Inventory.Weapons.Num()) % Inventory.Weapons.Num();
		EquipWeapon(Inventory.Weapons[IndexOfPrevWeapon]);
	}
}

int32 ASWFPSCharacter::GetPrimaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetPrimaryClipAmmo();
	}

	return 0;
}

int32 ASWFPSCharacter::GetMaxPrimaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetMaxPrimaryClipAmmo();
	}

	return 0;
}

int32 ASWFPSCharacter::GetPrimaryReserveAmmo() const
{
	if (CurrentWeapon && AmmoAttributeSet)
	{
		FGameplayAttribute Attribute = AmmoAttributeSet->GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType);
		if (Attribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(Attribute);
		}
	}

	return 0;
}

int32 ASWFPSCharacter::GetSecondaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetSecondaryClipAmmo();
	}

	return 0;
}

int32 ASWFPSCharacter::GetMaxSecondaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetMaxSecondaryClipAmmo();
	}

	return 0;
}

int32 ASWFPSCharacter::GetSecondaryReserveAmmo() const
{
	if (CurrentWeapon)
	{
		FGameplayAttribute Attribute = AmmoAttributeSet->GetReserveAmmoAttributeFromTag(CurrentWeapon->SecondaryAmmoType);
		if (Attribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(Attribute);
		}
	}

	return 0;
}

int32 ASWFPSCharacter::GetNumWeapons() const
{
	return Inventory.Weapons.Num();
}

FName ASWFPSCharacter::GetWeaponAttachPoint()
{
	return WeaponAttachPoint;
}

void ASWFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeFloatingStatusBar();

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerSyncCurrentWeapon();
	}
}

void ASWFPSCharacter::BindASCInput()
{
	if (!bASCInputBound && AbilitySystemComponent.IsValid() && IsValid(InputComponent))
	{
		FTopLevelAssetPath AbilityEnumAssetPath = FTopLevelAssetPath(FName("/Script/SWTypes"), FName("ESWAbilityInputID"));
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(FString("ConfirmTarget"),
			FString("CancelTarget"), AbilityEnumAssetPath, static_cast<int32>(ESWAbilityInputID::Confirm), static_cast<int32>(ESWAbilityInputID::Cancel)));

		bASCInputBound = true;
	}
}

void ASWFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASWFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InputActions->PrimaryFireAction, ETriggerEvent::Triggered, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::PrimaryFire, true);

		EnhancedInputComponent->BindAction(InputActions->PrimaryFireAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::PrimaryFire, false);

		EnhancedInputComponent->BindAction(InputActions->SecondaryFireAction, ETriggerEvent::Triggered, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::SecondaryFire, true);

		EnhancedInputComponent->BindAction(InputActions->SecondaryFireAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::SecondaryFire, false);

		EnhancedInputComponent->BindAction(InputActions->TakedownAction, ETriggerEvent::Started, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Takedown, true);

		EnhancedInputComponent->BindAction(InputActions->TakedownAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Takedown, false);
		
		EnhancedInputComponent->BindAction(InputActions->AlternateFireAction, ETriggerEvent::Started, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::AlternateFire, true);

		EnhancedInputComponent->BindAction(InputActions->AlternateFireAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::AlternateFire, false);

		EnhancedInputComponent->BindAction(InputActions->ReloadAction, ETriggerEvent::Started, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Reload, true);

		EnhancedInputComponent->BindAction(InputActions->ReloadAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Reload, false);

		EnhancedInputComponent->BindAction(InputActions->SprintAction, ETriggerEvent::Triggered, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Sprint, true);

		EnhancedInputComponent->BindAction(InputActions->SprintAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Sprint, false);

		EnhancedInputComponent->BindAction(InputActions->CrouchAction, ETriggerEvent::Started, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Crouch, true);

		EnhancedInputComponent->BindAction(InputActions->CrouchAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Crouch, false);

		EnhancedInputComponent->BindAction(InputActions->JumpAction, ETriggerEvent::Started, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Jump, true);

		EnhancedInputComponent->BindAction(InputActions->JumpAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Jump, false);

		EnhancedInputComponent->BindAction(InputActions->ConfirmAction, ETriggerEvent::Started, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Confirm, true);

		EnhancedInputComponent->BindAction(InputActions->ConfirmAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Confirm, false);

		EnhancedInputComponent->BindAction(InputActions->CancelAction, ETriggerEvent::Started, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Cancel, true);

		EnhancedInputComponent->BindAction(InputActions->CancelAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::Cancel, false);

		EnhancedInputComponent->BindAction(InputActions->NextWeaponAction, ETriggerEvent::Started, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::NextWeapon, true);

		EnhancedInputComponent->BindAction(InputActions->NextWeaponAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::NextWeapon, false);

		EnhancedInputComponent->BindAction(InputActions->PrevWeaponAction, ETriggerEvent::Started, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::PrevWeapon, true);

		EnhancedInputComponent->BindAction(InputActions->PrevWeaponAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::PrevWeapon, false);

		EnhancedInputComponent->BindAction(InputActions->PrevWeaponAction, ETriggerEvent::Completed, this,
			&ASWFPSCharacter::InvokeAbility, ESWAbilityInputID::PrevWeapon, false);

		EnhancedInputComponent->BindAction(InputActions->PauseAction, ETriggerEvent::Triggered, this,
			&ASWFPSCharacter::Pause);

		EnhancedInputComponent->BindAction(InputActions->RestartAction, ETriggerEvent::Triggered, this,
			&ASWFPSCharacter::RestartLevel);
	}
	//BindASCInput();
}

void ASWFPSCharacter::Pause()
{
	ASWPlayerController* SWPlayerController = Cast<ASWPlayerController>(GetWorld()->GetFirstPlayerController());
	if (SWPlayerController && !IsEndPlay)
	{
		SWPlayerController->TogglePauseMenu();
	}
}

void ASWFPSCharacter::RestartLevel()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("World is null"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("World is not null__________________________________"));
	}

	if (World)
	{
		FName CurrentLevelName = FName(*World->GetMapName());
        
		UGameplayStatics::OpenLevel(World, CurrentLevelName);
	}
}

void ASWFPSCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ASWFPSCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void ASWFPSCharacter::StartCrouch()
{
	Crouch();
}

void ASWFPSCharacter::StopCrouch()
{
	UnCrouch();
}

void ASWFPSCharacter::Jump()
{
	Super::Jump();
}

void ASWFPSCharacter::StopJumping()
{
	Super::StopJumping();
}

void ASWFPSCharacter::InvokeAbility(const FInputActionValue& Value, ESWAbilityInputID Id, bool IsActive)
{
	if (IsAlive())
	{
		SetLocalInputToASC(IsActive, Id);
	}
}

void ASWFPSCharacter::InitAbilitySystemComponent()
{
	ASWPlayerState* PS = GetPlayerState<ASWPlayerState>();
	if (PS)
	{
		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		AbilitySystemComponent = Cast<USWAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// AI won't have PlayerControllers so we can init again here just to be sure. No harm in initing twice for heroes that have PlayerControllers.
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		AttributeSet = PS->GetAttributeSet();
	}
}

void ASWFPSCharacter::SetLocalInputToASC(bool bIsPressed, const ESWAbilityInputID AbilityInputID)
{
	if(!AbilitySystemComponent.IsValid()) return;

	if(bIsPressed)
	{
		AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(AbilityInputID));
	}
	else
	{
		AbilitySystemComponent->AbilityLocalInputReleased(static_cast<int32>(AbilityInputID));
	}
}

