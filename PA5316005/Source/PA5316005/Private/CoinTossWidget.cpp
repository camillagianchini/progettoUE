// Fill out your copyright notice in the Description page of Project Settings.


#include "CoinTossWidget.h"
#include "Components/TextBlock.h"

void UCoinTossWidget::SetResultText(const FText& NewText)
{
	if (ResultTextBlock)
	{
		ResultTextBlock->SetText(NewText);
	}
}
