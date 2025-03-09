// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

/**
 * 
 */
UCLASS()
class STEAMWARS_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> SWSettingsMenuWidgetClass;
	
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* SettingsMenuWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> SWAuthorsMenuWidgetClass;
	
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* AuthorsMenuWidget;
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ExitGame();

	UFUNCTION(BlueprintCallable)
	void ShowSettings();

	UFUNCTION(BlueprintCallable)
	void ShowAuthors();
	
};
