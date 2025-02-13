#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SWCharacterMovementComponent.generated.h"

UCLASS()
class STEAMWARS_API USWCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USWCharacterMovementComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float SprintSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float ADSSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float CrouchSpeedMultiplier;

	uint8 RequestToStartSprinting : 1;
	uint8 RequestToStartADS : 1;
	uint8 RequestToStartCrouching : 1;

	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	// Sprint
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StartSprinting();
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StopSprinting();

	// Aim Down Sights
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
	void StartAimDownSights();
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
	void StopAimDownSights();

	UFUNCTION(BlueprintCallable, Category = "Crouch")
	void StartCrouching();
	UFUNCTION(BlueprintCallable, Category = "Crouch")
	void StopCrouching();
};
