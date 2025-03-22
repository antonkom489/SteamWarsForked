#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "EndGame.generated.h"

/**
 * 
 */
UCLASS()
class STEAMWARS_API UEndGame : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void RestartGame();

	UFUNCTION(BlueprintCallable)
	void SetGameOverText(FText NewText);

	UFUNCTION(BlueprintCallable)
	void SetGameOverTextBlock(UTextBlock* TextBlock);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Text")
	UTextBlock* GameOverTextBlock;  

	virtual void NativeConstruct() override; 
};