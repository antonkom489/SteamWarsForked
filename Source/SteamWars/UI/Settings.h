// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Settings.generated.h"

UCLASS()
class STEAMWARS_API USettings : public UUserWidget
{
	GENERATED_BODY()

	//void NativeConstruct();
public:
	// Установка чувствительности мыши
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMouseSens(float NewSensitivity);

	UFUNCTION(BlueprintCallable, Category = "Input")
	float GetMouseSens();

	// Инициализация чувствительности мыши
	UFUNCTION(BlueprintCallable, Category = "Input")
	void InitializeMouseSensitivity();

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	UPlayerInput* PlayerInput;

protected:
	// Чувствительность мыши по умолчанию
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	//float DefaultMouseSensitivity = 0.5f;
};
