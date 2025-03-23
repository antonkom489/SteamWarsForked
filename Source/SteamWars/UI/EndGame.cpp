#include "EndGame.h"
#include "Kismet/GameplayStatics.h"

void UEndGame::NativeConstruct()
{
	Super::NativeConstruct(); 
}

void UEndGame::RestartGame()
{
	UWorld* World = GetWorld();
	if (World)
	{
		FName CurrentLevelName = FName(*World->GetMapName());
        
		UGameplayStatics::OpenLevel(World, CurrentLevelName);
	}
}

void UEndGame::SetGameOverText(FText NewText)
{
	if (GameOverTextBlock)
	{
		GameOverTextBlock->SetText(NewText);
	}
}

void UEndGame::SetGameOverTextBlock(UTextBlock* TextBlock)
{
	GameOverTextBlock = TextBlock;
}