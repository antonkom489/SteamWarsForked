// Fill out your copyright notice in the Description page of Project Settings.

#include "Settings.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/GameplayStatics.h"



void USettings::InitializeMouseSensitivity()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		PlayerInput = PlayerController->PlayerInput;
	}
	SetMouseSens(GetMouseSens());
}

void USettings::SetMouseSens(float NewSensitivity)
{
	if (PlayerInput)
	PlayerInput->SetMouseSensitivity(NewSensitivity);
}

float USettings::GetMouseSens()
{
	return  PlayerInput->GetMouseSensitivityX();
}