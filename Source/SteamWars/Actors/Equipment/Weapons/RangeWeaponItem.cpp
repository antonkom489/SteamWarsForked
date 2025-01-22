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



