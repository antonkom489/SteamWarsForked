#include "SceneControl.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SWPlayerController.h"

void USceneControl::RestartGame()
{
	UWorld* World = GetWorld();
	if (World)
	{
		FName CurrentLevelName = FName(*World->GetMapName());
        
		UGameplayStatics::OpenLevel(World, CurrentLevelName);
	}
}

void USceneControl::TogglePause()
{
	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
	else
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}

void USceneControl::OnBackButtonClicked()
{
	ASWPlayerController* PlayerController = Cast<ASWPlayerController>(GetOwningPlayer());
	if (PlayerController)
	{
		PlayerController->HidePauseMenu();
	}
}

void USceneControl::ShowSettings()
{
	ASWPlayerController* PlayerController = Cast<ASWPlayerController>(GetOwningPlayer());
	if (PlayerController)
	{
		PlayerController->ShowSettingsMenu();
	}
}

void USceneControl::HideSettings()
{
	ASWPlayerController* PlayerController = Cast<ASWPlayerController>(GetOwningPlayer());
	if (PlayerController)
	{
		PlayerController->HideSettingsMenu();
	}
}