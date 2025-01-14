#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SWCharacterEquipmentComponent.generated.h"

enum class EEquipableItemType : uint8;
class ARangeWeaponItem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEAMWARS_API USWCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	EEquipableItemType GetCurrentEquippedItemType() const;

	void FireShoot();

	ARangeWeaponItem* GetCurrentWeapon() const { return CurrentEquippedWeapon; };
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
	TSubclassOf<ARangeWeaponItem> SideArmClass;

private:
	void CreateLoadout();
	
	ARangeWeaponItem* CurrentEquippedWeapon;
	TWeakObjectPtr<class ASWBaseCharacter> CachedBaseCharacter;
	const FName SocketCharacterWeapon = FName("CharacterWeaponSocket");
};
