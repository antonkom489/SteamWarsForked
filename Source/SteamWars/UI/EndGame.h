// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
};

