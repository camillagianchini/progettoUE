#include "CoinTossWidget.h"
#include "Components/TextBlock.h"

void UCoinTossWidget::SetCoinTossResult(const FString& ResultText)
{
    UE_LOG(LogTemp, Warning, TEXT("SetCoinTossResult called with: %s"), *ResultText);
    if (CoinTossResultText)
    {
        CoinTossResultText->SetText(FText::FromString(ResultText));
    }
}

