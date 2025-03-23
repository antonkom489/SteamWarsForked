#include "SWHUDWidget.h"

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
	}
}

void USWHUDWidget::ResetWeaponText()
{
	if (WeaponTextBlock)
	{
		WeaponTextBlock->SetText(FText::FromString(""));
	}
}
