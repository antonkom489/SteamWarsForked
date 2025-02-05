#include "RangeWeaponItem.h"

#include <random>

#include "Characters/SWBaseCharacter.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"
#include "Components/Weapon/WeaponBarrelComponent.h"
#include "SWTypes.h"
#include "Characters/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"
#include "Characters/CharacterComponents/AbilitySystem/Abilities/SWGameplayAbility.h"
#include "Characters/CharacterComponents/AbilitySystem/Abilities/SWGATA_LineTrace.h"
#include "Characters/CharacterComponents/AbilitySystem/Abilities/SWGATA_LineTraceWithBloom.h"
#include "Characters/CharacterComponents/AbilitySystem/Abilities/SWGATA_SphereTrace.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"


ARangeWeaponItem::ARangeWeaponItem()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	NetUpdateFrequency = 100.0f; // Set this to a value that's appropriate for your game
	bSpawnWithCollision = true;
	PrimaryClipAmmo = 0;
	MaxPrimaryClipAmmo = 0;
	SecondaryClipAmmo = 0;
	MaxSecondaryClipAmmo = 0;
	bInfiniteAmmo = false;
	PrimaryAmmoType = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));
	SecondaryAmmoType = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));

	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionComponent"));
	CollisionComp->InitCapsuleSize(40.0f, 50.0f);
	CollisionComp->SetCollisionObjectType(ECC_Pawn);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Manually enable when in pickup mode
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionComp;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->CastShadow = false;

	WeaponPrimaryInstantAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Weapon.Primary.Instant");
	WeaponSecondaryInstantAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Weapon.Secondary.Instant");
	WeaponAlternateInstantAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Weapon.Alternate.Instant");
	WeaponIsFiringTag = FGameplayTag::RequestGameplayTag("Weapon.IsFiring");

	FireMode = FGameplayTag::RequestGameplayTag("Weapon.FireMode.None");
	FullAutoFireMode = FGameplayTag::RequestGameplayTag("Weapon.FireMode.Complete.FullAuto");
	StatusText = DefaultStatusText;
	
	WeaponBarrel = CreateDefaultSubobject<UWeaponBarrelComponent>(TEXT("WeaponBarrel"));
	WeaponBarrel->SetupAttachment(WeaponMesh, SocketWeaponMuzzle);

	RestrictedPickupTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
	RestrictedPickupTags.AddTag(FGameplayTag::RequestGameplayTag("State.KnockedDown"));
	
}

class UAbilitySystemComponent* ARangeWeaponItem::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

USkeletalMeshComponent* ARangeWeaponItem::GetWeaponMesh() const
{
	return WeaponMesh;
}

void ARangeWeaponItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ARangeWeaponItem, OwningCharacter, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ARangeWeaponItem, PrimaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ARangeWeaponItem, MaxPrimaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ARangeWeaponItem, SecondaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ARangeWeaponItem, MaxSecondaryClipAmmo, COND_OwnerOnly);
}

void ARangeWeaponItem::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(ARangeWeaponItem, PrimaryClipAmmo, (IsValid(AbilitySystemComponent) && !AbilitySystemComponent->HasMatchingGameplayTag(WeaponIsFiringTag)));
	DOREPLIFETIME_ACTIVE_OVERRIDE(ARangeWeaponItem, SecondaryClipAmmo, (IsValid(AbilitySystemComponent) && !AbilitySystemComponent->HasMatchingGameplayTag(WeaponIsFiringTag)));
}

void ARangeWeaponItem::SetOwningCharacter(ASWFPSCharacter* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
	if (OwningCharacter)
	{
		// Called when added to inventory
		AbilitySystemComponent = Cast<USWAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());
		SetOwner(InOwningCharacter);
		AttachToComponent(OwningCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		AbilitySystemComponent = nullptr;
		SetOwner(nullptr);
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void ARangeWeaponItem::NotifyActorBeginOverlap(class AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	if (IsValidChecked(this) && !OwningCharacter)
	{
		PickUpOnTouch(Cast<ASWFPSCharacter>(Other));
	}
}

TSubclassOf<class USWHUDReticle> ARangeWeaponItem::GetPrimaryHUDReticleClass() const
{
	return PrimaryHUDReticleClass;
}

void ARangeWeaponItem::Equip()
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s %s OwningCharacter is nullptr"), *FString(__FUNCTION__), *GetName());
		return;
	}

	FName AttachPoint = OwningCharacter->GetWeaponAttachPoint();

	if (WeaponMesh1P)
	{
		WeaponMesh1P->AttachToComponent(OwningCharacter->GetFPSMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, AttachPoint);
		WeaponMesh1P->SetRelativeLocation(WeaponMesh1PEquippedRelativeLocation);
		WeaponMesh1P->SetRelativeRotation(WeaponMesh1PEquippedRelativeRotation);

		WeaponMesh1P->SetVisibility(true, true);

		if (WeaponAnimLinkLayer1P)
		{
			OwningCharacter->GetFPSMesh()->LinkAnimClassLayers(WeaponAnimLinkLayer1P);
		}
	}
}

void ARangeWeaponItem::UnEquip()
{
	if (OwningCharacter == nullptr)
	{
		return;
	}

	// Necessary to detach so that when toggling perspective all meshes attached won't become visible.

	WeaponMesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh1P->SetVisibility(false, true);
}

void ARangeWeaponItem::AddAbilities()
{
	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("%s %s Owning Role is null"), *FString(__FUNCTION__), *GetName());
		return;
	}

	USWAbilitySystemComponent* ASC = Cast<USWAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		//UE_LOG(LogTemp, Error, TEXT("%s %s Role: %s ASC is null"), *FString(__FUNCTION__), *GetName(), GET_ACTOR_ROLE_FSTRING(OwningCharacter));
		UE_LOG(LogTemp, Error, TEXT("%s %s ASC Role is null"), *FString(__FUNCTION__), *GetName());
		return;
	}

	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	for (TSubclassOf<USWGameplayAbility>& Ability : Abilities)
	{
		AbilitySpecHandles.Add(ASC->GiveAbility(
			FGameplayAbilitySpec(Ability, GetAbilityLevel(Ability.GetDefaultObject()->AbilityID), static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this)));
	}
}

void ARangeWeaponItem::RemoveAbilities()
{
	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	USWAbilitySystemComponent* ASC = Cast<USWAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		return;
	}

	// Remove abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	for (FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
	{
		ASC->ClearAbility(SpecHandle);
	}
}

int32 ARangeWeaponItem::GetAbilityLevel(ESWAbilityInputID AbilityID)
{
	return 1;
}

void ARangeWeaponItem::ResetWeapon()
{
	FireMode = DefaultFireMode;
	StatusText = DefaultStatusText;
}

int32 ARangeWeaponItem::GetPrimaryClipAmmo() const
{
	return PrimaryClipAmmo;
}

int32 ARangeWeaponItem::GetMaxPrimaryClipAmmo() const
{
	return MaxPrimaryClipAmmo;
}

int32 ARangeWeaponItem::GetSecondaryClipAmmo() const
{
	return SecondaryClipAmmo;
}

int32 ARangeWeaponItem::GetMaxSecondaryClipAmmo() const
{
	return MaxSecondaryClipAmmo;
}

void ARangeWeaponItem::SetPrimaryClipAmmo(int32 NewPrimaryClipAmmo)
{
	int32 OldPrimaryClipAmmo = PrimaryClipAmmo;
	PrimaryClipAmmo = NewPrimaryClipAmmo;
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
}

void ARangeWeaponItem::SetMaxPrimaryClipAmmo(int32 NewMaxPrimaryClipAmmo)
{
	int32 OldMaxPrimaryClipAmmo = MaxPrimaryClipAmmo;
	MaxPrimaryClipAmmo = NewMaxPrimaryClipAmmo;
	OnMaxPrimaryClipAmmoChanged.Broadcast(OldMaxPrimaryClipAmmo, MaxPrimaryClipAmmo);
}

void ARangeWeaponItem::SetSecondaryClipAmmo(int32 NewSecondaryClipAmmo)
{
	int32 OldSecondaryClipAmmo = SecondaryClipAmmo;
	SecondaryClipAmmo = NewSecondaryClipAmmo;
	OnSecondaryClipAmmoChanged.Broadcast(OldSecondaryClipAmmo, SecondaryClipAmmo);
}

void ARangeWeaponItem::SetMaxSecondaryClipAmmo(int32 NewMaxSecondaryClipAmmo)
{
	int32 OldMaxSecondaryClipAmmo = MaxSecondaryClipAmmo;
	MaxSecondaryClipAmmo = NewMaxSecondaryClipAmmo;
	OnMaxSecondaryClipAmmoChanged.Broadcast(OldMaxSecondaryClipAmmo, MaxSecondaryClipAmmo);
}

bool ARangeWeaponItem::HasInfiniteAmmo() const
{
	return bInfiniteAmmo;
}

UAnimMontage* ARangeWeaponItem::GetEquipMontage() const
{
	return Equip1PMontage;
}

void ARangeWeaponItem::OnDropped_Implementation(FVector NewLocation)
{
	SetOwningCharacter(nullptr);
	ResetWeapon();

	SetActorLocation(NewLocation);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	WeaponMesh1P->AttachToComponent(CollisionComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
	WeaponMesh1P->SetVisibility(false, true);
}

bool ARangeWeaponItem::OnDropped_Validate(FVector NewLocation)
{
	return true;
}

class USoundCue* ARangeWeaponItem::GetPickupSound() const
{
	return PickupSound;
}

FText ARangeWeaponItem::GetDefaultStatusText() const
{
	return DefaultStatusText;
}

ASWGATA_LineTrace* ARangeWeaponItem::GetLineTraceTargetActor()
{
	if (LineTraceTargetActor)
	{
		return LineTraceTargetActor;
	}

	LineTraceTargetActor = GetWorld()->SpawnActor<ASWGATA_LineTrace>();
	LineTraceTargetActor->SetOwner(this);
	return LineTraceTargetActor;
}

ASWGATA_LineTraceWithBloom* ARangeWeaponItem::GetLineTraceWithBloomTargetActor()
{
	if (LineTraceWithBloomTargetActor)
	{
		return LineTraceWithBloomTargetActor;
	}

	LineTraceWithBloomTargetActor = GetWorld()->SpawnActor<ASWGATA_LineTraceWithBloom>();
	LineTraceWithBloomTargetActor->SetOwner(this);
	return LineTraceWithBloomTargetActor;
}

ASWGATA_SphereTrace* ARangeWeaponItem::GetSphereTraceTargetActor()
{
	if (SphereTraceTargetActor)
	{
		return SphereTraceTargetActor;
	}

	SphereTraceTargetActor = GetWorld()->SpawnActor<ASWGATA_SphereTrace>();
	SphereTraceTargetActor->SetOwner(this);
	return SphereTraceTargetActor;
}

FVector ARangeWeaponItem::GetADSOffset() const
{
	return ADSOffset;
}

void ARangeWeaponItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsValid(OwningCharacter) || !IsValid(OwningCharacter->GetController()))
	{
		return;
	}

	if (bIsRecoilActive)
	{
		OwningCharacter->AddControllerPitchInput(RecoilPitchVelocity * -1.f * DeltaTime);
		OwningCharacter->AddControllerYawInput(RecoilYawVelocity * DeltaTime);

		RecoilPitchVelocity -= RecoilPitchDamping * DeltaTime;
		RecoilYawVelocity -= RecoilYawDamping * DeltaTime;

		if (RecoilPitchVelocity <= 0.0f)
		{
			bIsRecoilActive = false;
			StartRecoilRecovery();
		}
	}
	else if (bIsRecoilPitchRecoveryActive)
	{
		FRotator currentControlRotation = OwningCharacter->GetControlRotation();

		FRotator deltaRot = currentControlRotation - RecoilCheckpoint;
		deltaRot.Normalize();

		if (deltaRot.Pitch > 1.f)
		{
			float interpSpeed = (1.f / DeltaTime) / 4.f;
			FRotator interpRot = FMath::RInterpConstantTo(currentControlRotation, RecoilCheckpoint, DeltaTime, interpSpeed);
			interpSpeed = (1.f / DeltaTime) / 10.f; // TODO: figure out how to make dynamic yaw recovery speed that depends on the pitch distance so that the pitch and yaw recovery ands together smoothly
			interpRot.Yaw = FMath::RInterpTo(currentControlRotation, RecoilCheckpoint, DeltaTime, interpSpeed).Yaw;
			if (!bIsRecoilYawRecoveryActive)
			{
				interpRot.Yaw = currentControlRotation.Yaw;
			}

			OwningCharacter->GetController()->SetControlRotation(interpRot);
		}
		else if (deltaRot.Pitch > 0.1f)
		{
			float interpSpeed = (1.f / DeltaTime) / 6.f;
			FRotator interpRot = FMath::RInterpTo(currentControlRotation, RecoilCheckpoint, DeltaTime, interpSpeed);
			if (!bIsRecoilYawRecoveryActive)
			{
				interpRot.Yaw = currentControlRotation.Yaw;
			}
			OwningCharacter->GetController()->SetControlRotation(interpRot);
		}
		else
		{
			bIsRecoilPitchRecoveryActive = false;
			bIsRecoilYawRecoveryActive = false;
			bIsRecoilNeutral = true;
		}
	}

	if (CurrentTargetingSpread > 0.f)
	{
		float interpSpeed = (1.f / DeltaTime) / SpreadRecoveryInterpSpeed;
		CurrentTargetingSpread = FMath::FInterpConstantTo(CurrentTargetingSpread, 0.f, DeltaTime, interpSpeed);
	}
}

void ARangeWeaponItem::BeginPlay()
{
	ResetWeapon();
	if (!OwningCharacter && bSpawnWithCollision)
	{
		// Spawned into the world without an owner, enable collision as we are in pickup mode
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	Super::BeginPlay();
}

void ARangeWeaponItem::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (LineTraceTargetActor)
	{
		LineTraceTargetActor->Destroy();
	}

	if (SphereTraceTargetActor)
	{
		SphereTraceTargetActor->Destroy();
	}
	
	Super::EndPlay(EndPlayReason);
}

void ARangeWeaponItem::PickUpOnTouch(ASWFPSCharacter* InCharacter)
{
	if (!InCharacter || !InCharacter->IsAlive() || !InCharacter->GetAbilitySystemComponent() || InCharacter->GetAbilitySystemComponent()->HasAnyMatchingGameplayTags(RestrictedPickupTags))
	{
		return;
	}

	InCharacter->AddWeaponToInventory(this, true);

	/*if (InCharacter->AddWeaponToInventory(this, true) && OwningCharacter->IsInFirstPersonPerspective())
	{
		WeaponMesh3P->CastShadow = false;
		WeaponMesh3P->SetVisibility(true, true);
		WeaponMesh3P->SetVisibility(false, true);
	}*/
}

void ARangeWeaponItem::OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo)
{
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
}

void ARangeWeaponItem::OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo)
{
	OnMaxPrimaryClipAmmoChanged.Broadcast(OldMaxPrimaryClipAmmo, MaxPrimaryClipAmmo);
}

void ARangeWeaponItem::OnRep_SecondaryClipAmmo(int32 OldSecondaryClipAmmo)
{
	OnSecondaryClipAmmoChanged.Broadcast(OldSecondaryClipAmmo, SecondaryClipAmmo);
}

void ARangeWeaponItem::OnRep_MaxSecondaryClipAmmo(int32 OldMaxSecondaryClipAmmo)
{
	OnMaxPrimaryClipAmmoChanged.Broadcast(OldMaxSecondaryClipAmmo, MaxSecondaryClipAmmo);
}

float ARangeWeaponItem::SampleRecoilDirection(float x)
{
	return FMath::Sin((x + 5.f) * (2.f * PI / 20.f)) * (100.f - x);
}

void ARangeWeaponItem::StartRecoil()
{
	InitialRecoilPitchForce = BaseRecoilPitchForce;
	InitialRecoilYawForce = BaseRecoilYawForce;

	if (bIsUseADSStabilizer && FireMode == FullAutoFireMode)
	{
		CurrentADSHeat = OwningCharacter->GetADSAlpha() > 0.f ? CurrentADSHeat + 1.f : 0.f;
		float ADSHeatModifier = FMath::Clamp(CurrentADSHeat / MaxADSHeat, 0.f, ADSHeatModifierMax);
		InitialRecoilPitchForce *= 1.f - ADSHeatModifier;
		InitialRecoilYawForce *= 1.f - ADSHeatModifier;
	}

	RecoilPitchVelocity = InitialRecoilPitchForce;
	RecoilPitchDamping = RecoilPitchVelocity / 0.1f;

	std::random_device rd;
	std::mt19937 gen(rd());
	float directionStat = SampleRecoilDirection(RecoilStat);
	float directionScaleModifier = directionStat / 100.f;
	float stddev = InitialRecoilYawForce * (1.f - RecoilStat / 100.f);

	std::normal_distribution<float> d(InitialRecoilYawForce * directionScaleModifier, stddev);
	RecoilYawVelocity = d(gen);
	RecoilYawDamping = (RecoilYawVelocity * -1.f) / 0.1f;

	bIsRecoilActive = true;
}

void ARangeWeaponItem::StartRecoilRecovery()
{
	bIsRecoilPitchRecoveryActive = true;
	bIsRecoilYawRecoveryActive = true;
}

void ARangeWeaponItem::IncrementSpread()
{
	float maxSpread = FMath::Lerp(TargetingSpreadMax, TargetingSpreadMaxADS, OwningCharacter->GetADSAlpha());
	float spreadIncrement = FMath::Lerp(TargetingSpreadIncrement, TargetingSpreadIncrement * SpreadIncrementADSMod, OwningCharacter->GetADSAlpha());
	CurrentTargetingSpread = FMath::Min(maxSpread, CurrentTargetingSpread + spreadIncrement);
}

float ARangeWeaponItem::GetCurrentSpread() const
{
	return BaseSpread + CurrentTargetingSpread - FMath::Lerp(0.f, BaseSpread, OwningCharacter->GetADSAlpha());
}

void ARangeWeaponItem::ResetADSHeat()
{
	CurrentADSHeat = 0.f;
}

float ARangeWeaponItem::GetShotTimerInterval()
{
	return 60.0f / RateOfFire;
}



