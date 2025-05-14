#include "SWBaseAnimInstance.h"
#include "../FPSCharacter/SWFPSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
 
// Inverse of the minimum frame rate required to perform spring calculations.
#define MIN_DELTA_TIME_FOR_SPRING_CALCULATIONS 0.1f // 10 fps
 
/* Universal multiplier applied to spring model stiffness. Used to scale stiffness values to a more intuitive
* range for animators. */
#define SPRING_STIFFNESS_SCALER 35.0f

void USWBaseAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	checkf(TryGetPawnOwner()->IsA<ASWFPSCharacter>(), TEXT("USWBaseAnimInstance::NativeBeginPlay() USWBaseAnimInstance can be used only with APlayerCharacter"));
	CachedBaseCharacter = StaticCast<ASWFPSCharacter*>(TryGetPawnOwner());
}

void USWBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedBaseCharacter.IsValid()) return;

	UCharacterMovementComponent* CharacterMovement = CachedBaseCharacter->GetCharacterMovement();

	AimRotation2 = CachedBaseCharacter->GetBaseAimRotation();
	
	AController* Controller = CachedBaseCharacter->GetController();
	if(IsValid(Controller))
	{
		PlayerCameraPitchAngle = Controller->GetControlRotation().Pitch;
	}

	Speed = CachedBaseCharacter->GetVelocity().Size();
	bIsFalling = CharacterMovement->IsFalling();
	bIsCrouching = CharacterMovement->IsCrouching();
	/*-----------------TO-DO-----------------
	bIsSprinting = CharacterMovement->IsSprinting();
	bIsOutOfStamina = CharacterMovement->IsOutOfStamina();
	-----------------TO-DO-----------------*/
}

USWBaseAnimInstance::USWBaseAnimInstance()
{
	bUseMultiThreadedAnimationUpdate = true;
}

void USWBaseAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
 
	APawn* PawnOwner = TryGetPawnOwner();
	if (!PawnOwner)
	{
		return;
	}
 
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(PawnOwner->GetMovementComponent());
	if (!CharMovementComp || (CharMovementComp->MovementMode == MOVE_None))
	{
		return;
	}
 
	UpdateVelocityData();
	UpdateAimData(DeltaSeconds);
	UpdateMovementSwayData();
	UpdateAimSwayData();
	UpdateFallingOffsetData();
}

void USWBaseAnimInstance::UpdateVelocityData()
{
	APawn* PawnOwner = TryGetPawnOwner();
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(PawnOwner->GetMovementComponent());
 
	const FVector WorldVelocity = PawnOwner->GetVelocity();
	const FRotator WorldRotation = PawnOwner->GetActorRotation();
    
	// The character's "local" velocity is their world velocity relative to their world rotation.
	LocalVelocity = WorldRotation.UnrotateVector(WorldVelocity);
	LocalVelocity2D = LocalVelocity * FVector(1.0f, 1.0f, 0.0f);
    
	// Normalize the character's local velocity to their maximum movement speed.
	const float MaxMovementSpeed = CharMovementComp->GetMaxSpeed();
	const float NormalizedX = FMath::Clamp(UKismetMathLibrary::NormalizeToRange(LocalVelocity2D.X, 0.0f, MaxMovementSpeed), -1.0f, 1.0f);
	const float NormalizedY = FMath::Clamp(UKismetMathLibrary::NormalizeToRange(LocalVelocity2D.Y, 0.0f, MaxMovementSpeed), -1.0f, 1.0f);
	LocalVelocity2DNormalized = FVector(NormalizedX, NormalizedY, 0.0f);
}

void USWBaseAnimInstance::UpdateAimData(float DeltaSeconds)
{
	const FRotator PreviousAimRotation = AimRotation;
    
	AimRotation = TryGetPawnOwner()->GetBaseAimRotation();
	AimRotation.Pitch = FRotator::NormalizeAxis(AimRotation.Pitch); // Fix for a problem with how UE replicates aim rotation.
    
	// Use a normalized delta to account for winding (e.g. 359.0 -> 1.0 should be 2.0, not -358.0).
	const FRotator RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, PreviousAimRotation);
    
	const float InverseDeltaSeconds = ((DeltaSeconds > 0.0f) ? (1.0f / DeltaSeconds) : 0.0f); // Avoid dividing by 0.
    
	AimSpeedRightLeft = RotationDelta.Yaw * InverseDeltaSeconds;
	AimSpeedUpDown = RotationDelta.Pitch * InverseDeltaSeconds;
}

void USWBaseAnimInstance::UpdateMovementSwayData()
{
	// Use the owning pawn's maximum movement speed as the bound for movement sway.
	const float MaxMovementSpeed = TryGetPawnOwner()->GetMovementComponent()->GetMaxSpeed();
 
	// Calculate the forward/backward movement spring target.
	const float ClampedSpeedX = FMath::Clamp(LocalVelocity2D.X, -MaxMovementSpeed, MaxMovementSpeed);
	const float SpringTargetForwardBackward = UKismetMathLibrary::NormalizeToRange((ClampedSpeedX * MoveSwayForwardBackwardSpringModelData.InterpSpeed), 0.0f, MaxMovementSpeed);
 
	// Don't bother performing spring calculations at low frame-rates.
	const float DeltaSeconds = GetDeltaSeconds();
	if (DeltaSeconds > MIN_DELTA_TIME_FOR_SPRING_CALCULATIONS)
	{
		return;
	}
 
	/* Apply an arbitrary multiplier to the spring's stiffness value. This scales viable spring stiffness values to a
	 * more intuitive range of (0 -> 100) when adjusting spring model data. */
	const float EffectiveStiffness = MoveSwayForwardBackwardSpringModelData.Stiffness * SPRING_STIFFNESS_SCALER;
 
	// Perform the spring calculation with the given data.
	CurrentSpringMoveForwardBackward = UKismetMathLibrary::FloatSpringInterp
	(
		CurrentSpringMoveForwardBackward,
		SpringTargetForwardBackward,
		SpringStateMoveForwardBackward,
		EffectiveStiffness,
		MoveSwayForwardBackwardSpringModelData.CriticalDampingFactor,
		DeltaSeconds,
		MoveSwayForwardBackwardSpringModelData.Mass,
		1.0f,
		false
	);
}

void USWBaseAnimInstance::UpdateAimSwayData()
{
	const float MaxAimSpeed = 720.0f;
	const float MaxAimSpeedUpDown = (MaxAimSpeed / 2.0f); // Halved because characters' pitch has half the range of their yaw: (-90 -> 90) vs. (0 -> 360).
    
	// Calculate the right/left aim sway spring.
	const float ClampedSpeedRightLeft = FMath::Clamp(AimSpeedRightLeft, -MaxAimSpeed, MaxAimSpeed);
	const float SpringTargetRightLeft = UKismetMathLibrary::NormalizeToRange((ClampedSpeedRightLeft * AimSwayRightLeftSpringModelData.InterpSpeed), 0.0f, MaxAimSpeed);
    
	CurrentSpringAimRightLeft = UpdateFloatSpringInterp
	(
		CurrentSpringAimRightLeft,
		SpringTargetRightLeft,
		SpringStateAimRightLeft,
		AimSwayRightLeftSpringModelData
	);
    
	// Calculate the up/down aim sway spring.
	const float ClampedSpeedUpDown = FMath::Clamp(AimSpeedUpDown, -MaxAimSpeedUpDown, MaxAimSpeedUpDown);
	const float SpringTargetUpDown = UKismetMathLibrary::NormalizeToRange((ClampedSpeedUpDown * AimSwayUpDownSpringModelData.InterpSpeed), 0.0f, MaxAimSpeedUpDown);
    
	CurrentSpringAimUpDown = UpdateFloatSpringInterp
	(
		CurrentSpringAimUpDown,
		SpringTargetUpDown,
		SpringStateAimUpDown,
		AimSwayUpDownSpringModelData
	);
}

void USWBaseAnimInstance::UpdateFallingOffsetData()
{
	// Use the owning pawn's jump velocity as the bound for the falling offset.
	const float MaxVerticalSpeed = Cast<UCharacterMovementComponent>(TryGetPawnOwner()->GetMovementComponent())->JumpZVelocity;
 
	// Calculate the falling offset spring.
	float SpringTargetFalling = UKismetMathLibrary::NormalizeToRange(LocalVelocity.Z, 0.0f, MaxVerticalSpeed);
 
	CurrentSpringFalling = UpdateFloatSpringInterp
	(
		CurrentSpringFalling,
		SpringTargetFalling,
		SpringStateFalling,
		FallingOffsetSpringModelData
	);
}

float USWBaseAnimInstance::UpdateFloatSpringInterp(float SpringCurrent, float SpringTarget,
	FFloatSpringState& SpringState, FFloatSpringModelData& SpringData) const
{
	const float DeltaSeconds = GetDeltaSeconds();
    
	// Don't bother performing spring calculations at low frame-rates.
	if (DeltaSeconds > MIN_DELTA_TIME_FOR_SPRING_CALCULATIONS)
	{
		return SpringCurrent;
	}
    
	/* Apply an arbitrary multiplier to the spring's stiffness value. This scales viable spring stiffness values to a
	 * more intuitive range of (0 - 100) when adjusting spring model data. */
	const float EffectiveStiffness = SpringData.Stiffness * SPRING_STIFFNESS_SCALER;
    
	// Perform the spring calculation with the given data.
	return UKismetMathLibrary::FloatSpringInterp
	(
		SpringCurrent,
		SpringTarget,
		SpringState,
		EffectiveStiffness,
		SpringData.CriticalDampingFactor,
		DeltaSeconds,
		SpringData.Mass,
		1.0f,
		false
	);
}
