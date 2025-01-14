#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquipableItem.h"
#include "RangeWeaponItem.generated.h"

class UWeaponBarrelComponent;

UCLASS(Blueprintable)
class STEAMWARS_API ARangeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	ARangeWeaponItem();

	void FireShoot();

	UWeaponBarrelComponent* GetWeaponBarrelComponent() const { return WeaponBarrel; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWeaponBarrelComponent* WeaponBarrel;

private:
	const FName SocketWeaponMuzzle = FName("Barrel");
};
