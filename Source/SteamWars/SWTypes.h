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
	Four
};

UENUM(BlueprintType)
enum class ESWAbilityInputID : uint8
{
	None,
	Confirm,
	Cancel,
	Shoot,
	ShootAI
};

UENUM(BlueprintType)
enum class EGDHitReactDirection : uint8
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