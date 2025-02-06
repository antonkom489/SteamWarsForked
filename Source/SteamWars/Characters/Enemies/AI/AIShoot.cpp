#include "AIShoot.h"

#include "AIController.h"
#include "Characters/CharacterComponents/AbilitySystem/SWAbilitySystemComponent.h"

EBTNodeResult::Type UAIShoot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Получаем AI Controller
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		UE_LOG(LogTemp, Error, TEXT("AIController is null"));
		return EBTNodeResult::Failed;
	}

	// Получаем Pawn
	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("ControlledPawn is null"));
		return EBTNodeResult::Failed;
	}

	// Получаем AbilitySystemComponent
	USWAbilitySystemComponent* AbilitySystemComponent = ControlledPawn->FindComponentByClass<USWAbilitySystemComponent>();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AbilitySystemComponent is null"));
		return EBTNodeResult::Failed;
	}
	
	// Проверяем, можно ли активировать способность с указанным тегом
	if (AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag), false))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully activated ability with tag: %s"), *AbilityTag.ToString());
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to activate ability with tag: %s"), *AbilityTag.ToString());
		UE_LOG(LogTemp, Error, TEXT("Shoot!"));
		return EBTNodeResult::Failed;
	}
}
