#pragma once

#include "CoreMinimal.h"
#include "../SWBaseCharacter.h"
#include "EnemyBaseCharacter.generated.h"

class UBehaviorTree;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackEnd, FTimerHandle, TimerHandle);

UCLASS()
class STEAMWARS_API AEnemyBaseCharacter : public ASWBaseCharacter
{
	GENERATED_BODY()

public:
	AEnemyBaseCharacter(const class FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyDied OnEnemyDied;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Events")
	FOnAttackEnd OnAttackEnd;
	
	virtual void FinishDying() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Events")
	void Threated(FVector ThreatedSource);
	virtual void Threated_Implementation(FVector ThreatedSource);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Events")
	void EnterCover();
	virtual void EnterCover_Implementation();

	UFUNCTION(BlueprintCallable)
	void ShootAI();

	UFUNCTION(BlueprintCallable)
	void ShotAI();

	UFUNCTION(BlueprintCallable)
	UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	
protected:
	virtual void BeginPlay() override;
	
	// Actual hard pointer to AbilitySystemComponent
	UPROPERTY()
	class USWAbilitySystemComponent* HardRefAbilitySystemComponent;

	// Actual hard pointer to AttributeSetBase
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Events")
	class USWAttributeSet* HardRefAttributeSet;
	
	FDelegateHandle HealthChangedDelegateHandle;

	// Attribute changed callbacks
	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	// Tag change callbacks
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Events")
	UBehaviorTree* BehaviorTree;


private:
	bool bIsInCover = false;
};
