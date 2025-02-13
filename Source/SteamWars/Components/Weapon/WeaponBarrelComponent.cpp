#include "WeaponBarrelComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "SWTypes.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"


UWeaponBarrelComponent::UWeaponBarrelComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

