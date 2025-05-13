#include "SWHUDWidget.h"
#include "LevelSequencePlayer.h"

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
			SequencePlayer->Play();
		}
	}
}
