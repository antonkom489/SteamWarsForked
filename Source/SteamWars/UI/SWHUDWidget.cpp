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
		
		OnWeaponTextChanged.Broadcast(NewText);
	}
}