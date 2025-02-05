#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "EquipableItem.generated.h"

UENUM(BlueprintType)
enum class EEquipableItemType : uint8
{
	None,
	Pistol
};

UCLASS(Abstract, NotBlueprintable)
class STEAMWARS_API AEquipableItem : public AActor
{
	GENERATED_BODY()

public:
	EEquipableItemType GetItemType() const;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable Item")
	EEquipableItemType ItemType;
};
