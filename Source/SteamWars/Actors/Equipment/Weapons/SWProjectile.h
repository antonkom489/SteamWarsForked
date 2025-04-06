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

	// Функция для инициализации снаряда
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void Initialize(FVector TargetLocation);

protected:
	// Компонент для движения снаряда
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PBProjectile")
	class UProjectileMovementComponent* ProjectileMovement;
};
