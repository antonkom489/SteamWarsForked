#include "SWBaseAnimInstance.h"
#include "../FPSCharacter/SWFPSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	AimRotation = CachedBaseCharacter->GetBaseAimRotation();
	
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
