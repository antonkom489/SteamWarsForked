#include "SWCharacterMovementComponent.h"

#include "../SWBaseCharacter.h"

USWCharacterMovementComponent::USWCharacterMovementComponent()
{
	SprintSpeedMultiplier = 1.4f;
	ADSSpeedMultiplier = 0.55f;
	CrouchSpeedMultiplier = 0.65;
}

float USWCharacterMovementComponent::GetMaxSpeed() const
{
	ASWBaseCharacter* Owner = Cast<ASWBaseCharacter>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() No Owner"), *FString(__FUNCTION__));
		return Super::GetMaxSpeed();
	}

	if (!Owner->IsAlive())
	{
		return 0.0f;
	}	

	if (RequestToStartSprinting)
	{
		return Owner->GetMoveSpeed() * SprintSpeedMultiplier;
	}

	// ADS and crouch can happen simultanously, so we only return the slowest of the two (ADS) when both are requested
	if (RequestToStartADS)
	{
		return Owner->GetMoveSpeed() * ADSSpeedMultiplier;
	}

	if (RequestToStartCrouching)
	{
		return Owner->GetMoveSpeed() * CrouchSpeedMultiplier;
	}

	return Owner->GetMoveSpeed();
}

void USWCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	RequestToStartSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;

	RequestToStartADS = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

void USWCharacterMovementComponent::StartSprinting()
{
	RequestToStartSprinting = true;
}

void USWCharacterMovementComponent::StopSprinting()
{
	RequestToStartSprinting = false;
}

void USWCharacterMovementComponent::StartAimDownSights()
{
	RequestToStartADS = true;
}

void USWCharacterMovementComponent::StopAimDownSights()
{
	RequestToStartADS = false;
}

void USWCharacterMovementComponent::StartCrouching()
{
	RequestToStartCrouching = true;
}

void USWCharacterMovementComponent::StopCrouching()
{
	RequestToStartCrouching = false;
}
