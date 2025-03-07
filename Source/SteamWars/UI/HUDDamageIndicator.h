#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDDamageIndicator.generated.h"


UCLASS()
class STEAMWARS_API UHUDDamageIndicator : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UPROPERTY(BlueprintReadOnly)
    FVector HitLocation;
    
    UFUNCTION(BlueprintCallable)
    float UpdateAngle(FVector ForwardVector, FVector ActorLocationVector);
};
