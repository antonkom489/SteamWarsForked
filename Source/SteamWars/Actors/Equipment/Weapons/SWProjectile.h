#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWProjectile.generated.h"

UCLASS()
class STEAMWARS_API ASWProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASWProjectile();

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PBProjectile")
	class UProjectileMovementComponent* ProjectileMovement;
};
