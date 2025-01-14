#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "../SWBaseCharacter.h"
#include "SWFPSCharacter.generated.h"

enum class ESWAbilityInputID : uint8;

UCLASS()
class STEAMWARS_API ASWFPSCharacter : public ASWBaseCharacter
{
	GENERATED_BODY()

public:
	ASWFPSCharacter();

	// GAS Server only
	virtual void PossessedBy(AController* NewController) override;
	
	// GAS Client only
	virtual void OnRep_PlayerState() override;
	
	USkeletalMeshComponent* GetFPSMesh() const { return FirstPersonMeshComponent; }

	void FairShoot();

	virtual void FinishDying() override;
	
protected:
	virtual void BeginPlay() override;

	/*-------------------Components-------------------*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	class USkeletalMeshComponent* FirstPersonMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	class UInputDataAsset* InputActions;
	/*------------------------------------------------*/

	FGameplayTag DeadTag;
	
public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/*---------------Movement---------------*/
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartCrouch();
	void StopCrouch();
	virtual void Jump() override;
	virtual void StopJumping() override;
	/*---------------------------------------*/
	void FairShootHandle();
	

	void InitAbilitySystemComponent();

	void SetLocalInputToASC(bool bIsPressed, const ESWAbilityInputID AbilityInputID);
};
