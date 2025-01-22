#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SWBaseAnimInstance.generated.h"

UCLASS()
class STEAMWARS_API USWBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
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
	FRotator AimRotation = FRotator::ZeroRotator;

	/*-----------------TO-DO-----------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsSprining = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsOutOfStamina = false;
	-----------------TO-DO-----------------*/

private:
	TWeakObjectPtr<class ASWFPSCharacter> CachedBaseCharacter;
};
