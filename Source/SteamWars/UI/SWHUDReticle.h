// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SWHUDReticle.generated.h"

UCLASS()
class STEAMWARS_API USWHUDReticle : public UUserWidget
{
	GENERATED_BODY()

public:
	// Essentially an interface that all child classes in Blueprint will have to fill out
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetColor(FLinearColor Color);
};
