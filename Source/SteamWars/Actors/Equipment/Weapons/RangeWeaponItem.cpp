#include "RangeWeaponItem.h"

#include "Characters/SWBaseCharacter.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"
#include "Components/Weapon/WeaponBarrelComponent.h"


ARangeWeaponItem::ARangeWeaponItem()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	WeaponBarrel = CreateDefaultSubobject<UWeaponBarrelComponent>(TEXT("WeaponBarrel"));
	WeaponBarrel->SetupAttachment(WeaponMesh, SocketWeaponMuzzle);
}

void ARangeWeaponItem::FireShoot()
{
	if(WeaponBarrel)
	{
		checkf(GetOwner()->IsA<ASWFPSCharacter>(), TEXT("ARangeWeaponItem::FireShoot() only ASWFPSCharacter can be an owner of range weapon"));
		ASWFPSCharacter* CharacterOwner = StaticCast<ASWFPSCharacter*>(GetOwner());

		APlayerController* Controller = CharacterOwner->GetController<APlayerController>();

		if(!IsValid(Controller)) return;

		FVector PlayerViewPoint;
		FRotator PlayerViewRotation;
		
		Controller->GetPlayerViewPoint(PlayerViewPoint, PlayerViewRotation);

		FVector ViewDirection = PlayerViewRotation.RotateVector(FVector::ForwardVector);
		
		WeaponBarrel->Shot(PlayerViewPoint, ViewDirection);
	}
}



