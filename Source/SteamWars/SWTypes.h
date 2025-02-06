#pragma once
#include "CoreMinimal.h"

#define ECC_Bullet ECC_GameTraceChannel1

UENUM(BlueprintType)
enum class ESpawnersID : uint8
{
	None,
	One,
	Two,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eith,
	Nine,
	Ten
};

UENUM(BlueprintType)
enum class ESWAbilityInputID : uint8
{
	None,
	Confirm,
	Cancel,
	PrimaryFire,
	SecondaryFire,
	AlternateFire,
	Takedown,
	Reload,
	Sprint,
	Crouch,
	Jump,
	NextWeapon,
	PrevWeapon,
	Shoot,
	ShootAI,
	ShotAI
};

UENUM(BlueprintType)
enum class ESWHitReactDirection : uint8
{
	// 0
	None			UMETA(DisplayName = "None"),
	// 1
	Left 			UMETA(DisplayName = "Left"),
	// 2
	Front 			UMETA(DisplayName = "Front"),
	// 3
	Right			UMETA(DisplayName = "Right"),
	// 4
	Back			UMETA(DisplayName = "Back")
};

const FName SocketFPCamera = FName("CameraSocket");
const FName SocketWeaponMuzzle = FName("Barrel");