#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "SWBaseAnimInstance.generated.h"

/**
* Defines the behavior of a spring model. Used for performing calculations for sway animations.
*/
USTRUCT(BlueprintType)
struct FFloatSpringModelData
{
	GENERATED_BODY()
 
	/* Controls the amplitude of the spring model. This value is signed, so setting it to a negative number reverses the
	 * direction of the spring (e.g. to create the effect of leading versus lagging). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = -10.0f, UIMin = -10.0f, ClampMax = 10.0f, UIMax = 10.0f))
	float InterpSpeed = 1.0f;
 
	/* Represents the stiffness of this spring. Higher values reduce overall oscillation. Scales with Mass (i.e. a lower
	 * mass will make the spring appear less stiff). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 100.0f, UIMax = 100.0f))
	float Stiffness = 25.0f;
 
	/* The amount of damping applied to the spring. 0.0 means no damping (full oscillation), 1.0 means full damping
	 * (no oscillation). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 1.0f, UIMax = 1.0f))
	float CriticalDampingFactor = 0.5f;
 
	// A multiplier that simulates the spring's, affecting the amount of force required to oscillate it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = 1.0f, UIMin = 1.0f, ClampMax = 100.0f, UIMax = 100.0f))
	float Mass = 10.0f;
};

UCLASS()
class STEAMWARS_API USWBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	USWBaseAnimInstance();
 
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animations | FP")
	float PlayerCameraPitchAngle = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animations")
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animations")
	bool bIsFalling = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animations")
	bool bIsCrouching = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animations")
	FRotator AimRotation2 = FRotator::ZeroRotator;

	// Calculate velocity data this frame.
	void UpdateVelocityData();
 
	// Calculate aim data this frame.
	void UpdateAimData(float DeltaSeconds);
 
	// Updates movement sway data using a spring model.
	void UpdateMovementSwayData();
 
	// Updates aim sway data using a spring model.
	void UpdateAimSwayData();
 
	// Updates falling offset data using a spring model.
	void UpdateFallingOffsetData();

	// This character's current velocity, relative to its world rotation.
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	FVector LocalVelocity;
 
	// This character's current local velocity with vertical velocity (Z) masked out.
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	FVector LocalVelocity2D;
 
	/* This character's current local velocity, normalized to its maximum movement speed. Vertical velocity (Z) is
	 * masked out. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	FVector LocalVelocity2DNormalized;

	// This character's current base aim rotation.
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data")
	FRotator AimRotation;
    
	// The normalized rate at which the owning character's aim yaw is changing, in degrees/second.
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data", DisplayName = "Aim Speed (Right/Left)")
	float AimSpeedRightLeft;
    
	// The normalized rate at which the owning character's aim pitch is changing, in degrees/second.
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data", DisplayName = "Aim Speed (Up/Down)")
	float AimSpeedUpDown;

	// The spring model used to drive forward/backward movement sway for this animation instance.
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Movement Sway", DisplayName = "Forward/Backward Sway Spring Model")
	FFloatSpringModelData MoveSwayForwardBackwardSpringModelData;
 
	// The spring model used to drive right/left movement sway for this animation instance.
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Movement Sway", DisplayName = "Right/Left Sway Spring Model")
	FFloatSpringModelData MoveSwayRightLeftSpringModelData;

	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Movement Sway", DisplayName = "Current Movement Sway Value (Forward/Backward)")
	float CurrentSpringMoveForwardBackward;
 
	// The current spring value of the right/left movement sway spring.
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Movement Sway", DisplayName = "Current Movement Sway Value (Right/Left)")
	float CurrentSpringMoveRightLeft;

	// The spring model used to drive right/left aim sway for this animation instance.
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Aim Sway", DisplayName = "Right/Left Sway Spring Model")
	FFloatSpringModelData AimSwayRightLeftSpringModelData;
 
	// The spring model used to drive up/down aim sway for this animation instance.
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Aim Sway", DisplayName = "Up/Down Sway Spring Model")
	FFloatSpringModelData AimSwayUpDownSpringModelData;
	
	// The current spring value for the right/left aim sway spring.
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Aim Sway", DisplayName = "Current Aim Sway Value (Right/Left)")
	float CurrentSpringAimRightLeft;
 
	// The current spring value for the up/down aim sway spring.
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Aim Sway", DisplayName = "Current Aim Sway Value (Up/Down)")
	float CurrentSpringAimUpDown;
	
	// The spring model used to drive the falling (vertical movement) offset for this animation instance.
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Falling Offset", DisplayName = "Falling Offset Spring Model")
	FFloatSpringModelData FallingOffsetSpringModelData;
 
	// The current spring value of the falling offset spring.
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Falling Offset", DisplayName = "Current Falling Offset Value")
	float CurrentSpringFalling;
	
	/*-----------------TO-DO-----------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsSprining = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsOutOfStamina = false;
	-----------------TO-DO-----------------*/

	float UpdateFloatSpringInterp(float SpringCurrent, float SpringTarget, FFloatSpringState& SpringState, FFloatSpringModelData& SpringData) const;
	
private:
	TWeakObjectPtr<class ASWFPSCharacter> CachedBaseCharacter;

	// Spring state for the forward/backward movement sway's spring calculations.
	FFloatSpringState SpringStateMoveForwardBackward;
 
	// Spring state for the right/left movement sway's spring calculations.
	FFloatSpringState SpringStateMoveRightLeft;

	// Spring state for the right/left aim sway's spring calculations.
	FFloatSpringState SpringStateAimRightLeft;
 
	// Spring state for the up/down aim sway's spring calculations.
	FFloatSpringState SpringStateAimUpDown;
	
	// Spring state for the falling offset's spring calculations.
	FFloatSpringState SpringStateFalling;

	
	
};
