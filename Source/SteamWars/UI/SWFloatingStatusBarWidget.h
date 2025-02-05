#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SWFloatingStatusBarWidget.generated.h"


UCLASS()
class STEAMWARS_API USWFloatingStatusBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	class ASWBaseCharacter* OwningCharacter;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHealthPercentage(float HealthPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCharacterName(const FText& NewName);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PlayFadeIn();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PlayFadeOut();
};
