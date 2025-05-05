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
	{
		PlayerInput->SetMouseSensitivity(NewSensitivity);
		GConfig->SetFloat(TEXT("/Script/Game.Settings"), TEXT("MouseSensitivity"), NewSensitivity, GGameIni);
		GConfig->Flush(false, GGameIni);
	}
}


float USettings::GetMouseSens()
{
	float Sens = 1.0f;
	GConfig->GetFloat(TEXT("/Script/Game.Settings"), TEXT("MouseSensitivity"), Sens, GGameIni);
	return Sens;
}
