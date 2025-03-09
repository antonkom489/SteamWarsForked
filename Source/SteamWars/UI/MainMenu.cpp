// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"

void UMainMenu::ExitGame()
{
	FGenericPlatformMisc::RequestExit(false);
}

void UMainMenu::ShowSettings()
{
	if (SWSettingsMenuWidgetClass)
	{
		SettingsMenuWidget = CreateWidget<UUserWidget>(this, SWSettingsMenuWidgetClass);
		if (SettingsMenuWidget)
		{
			SettingsMenuWidget->AddToViewport();
		}
	}
}

void UMainMenu::ShowAuthors()
{
	if (SWAuthorsMenuWidgetClass)
	{
		AuthorsMenuWidget = CreateWidget<UUserWidget>(this, SWAuthorsMenuWidgetClass);
		if (AuthorsMenuWidget)
		{
			AuthorsMenuWidget->AddToViewport();
		}
	}
}