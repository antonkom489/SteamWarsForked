#pragma once

#include "CoreMinimal.h"
#include "SWTypes.h"
#include "Abilities/GameplayAbility.h"
#include "SWGameplayAbility.generated.h"

enum class ESWAbilityInputID : uint8;

UCLASS()
class STEAMWARS_API USWGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	USWGameplayAbility();

	//Abilities will activate when input is pressed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	ESWAbilityInputID AbilityInputID = ESWAbilityInputID::None;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	ESWAbilityInputID AbilityID = ESWAbilityInputID::None;

	// Tells an ability to activate immediately when its granted. Used for passive abilities and abilities forced on others.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool ActivateAbilityOnGranted = false;

	// If an ability is marked as 'ActivateAbilityOnGranted', activate them immediately when given here
	// Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
};
