#include "SWHUDWidget.h"
#include "LevelSequencePlayer.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"
#include "Player/SWPlayerController.h"

void USWHUDWidget::SetWeaponTextBlock(UTextBlock* TextBlock)
{
	WeaponTextBlock = TextBlock;
}

void USWHUDWidget::SetWeaponText(FText NewText)
{
	if (WeaponTextBlock)
	{
		WeaponTextBlock->SetText(NewText);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USWHUDWidget::ResetWeaponText,
			ResetTime, false);

		PlaySequence();
	}
}

void USWHUDWidget::ResetWeaponText()
{
	if (WeaponTextBlock)
	{
		WeaponTextBlock->SetText(FText::FromString(""));
	}
}

void USWHUDWidget::PlaySequence()
{
	if (MySequence)
	{		
		ALevelSequenceActor* SequenceActor = GetWorld()->SpawnActor<ALevelSequenceActor>(ALevelSequenceActor::StaticClass());
		SequenceActor->SetSequence(MySequence);
        
		// Получаем плеер последовательности и запускаем воспроизведение
		ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
			GetWorld(),
			MySequence,
			FMovieSceneSequencePlaybackSettings(),
			SequenceActor
		);
		
		if (SequencePlayer)
		{
			SequencePlayer->OnPlay.AddDynamic(this, &USWHUDWidget::DisablePlayerInput);
			FTimerHandle PlayDelayHandle;
			GetWorld()->GetTimerManager().SetTimer( PlayDelayHandle, [SequencePlayer]()
				{
					if (SequencePlayer)
					{
						SequencePlayer->Play();
					}
				},
				PlayDelay,
				false
			);
			SequencePlayer->OnFinished.AddDynamic(this, &USWHUDWidget::OnSequenceFinished);
		}
	}
}

void USWHUDWidget::OnSequenceFinished()
{
	if (GetWorld())
	{
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController)
		{
			PlayerController->EnableInput(nullptr);
			PlayerController->GetPawn()->EnableInput(nullptr);
		}
	}
}

void USWHUDWidget::DisablePlayerInput()
{
	if (GetWorld())
	{
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController)
		{
			PlayerController->DisableInput(nullptr);
			PlayerController->GetPawn()->DisableInput(nullptr);
		}

		ASWFPSCharacter* FPSCharacter = Cast<ASWFPSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (IsValid(FPSCharacter))
		{
			FPSCharacter->OnXrayReleased();
		}
	}
}

