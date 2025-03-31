#include "CoinTossWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/KismetMathLibrary.h"



void UCoinTossWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TossButton)
	{
		TossButton->OnClicked.AddDynamic(this, &UCoinTossWidget::OnTossButtonClicked);
	}
}

void UCoinTossWidget::OnTossButtonClicked()
{

    if (bTossExecuted)
    {
        return;
    }

	ExecuteCoinToss();
}

void UCoinTossWidget::ExecuteCoinToss()
{
    int32 CoinResult = UKismetMathLibrary::RandomIntegerInRange(0, 1);
    FString ResultString = (CoinResult == 0) ? TEXT("Human") : TEXT("AI");

    if (ResultText)
    {
        ResultText->SetText(FText::FromString(ResultString));
        //UE_LOG(LogTemp, Warning, TEXT("Coin toss result: %s"), *ResultString);
    }

    OnCoinTossCompleted.Broadcast(CoinResult);
}
