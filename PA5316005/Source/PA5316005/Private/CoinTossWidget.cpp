#include "CoinTossWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/KismetMathLibrary.h"



void UCoinTossWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Assicuriamoci che il bottone esista
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

	// Quando il bottone viene cliccato, eseguiamo il coin toss
	ExecuteCoinToss();
}

void UCoinTossWidget::ExecuteCoinToss()
{
    // Genera il coin toss (0 o 1)
    int32 CoinResult = UKismetMathLibrary::RandomIntegerInRange(0, 1);
    FString ResultString = (CoinResult == 0) ? TEXT("Human") : TEXT("AI");

    // Aggiorna il TextBlock con il risultato e stampa il log
    if (ResultText)
    {
        ResultText->SetText(FText::FromString(ResultString));
        UE_LOG(LogTemp, Warning, TEXT("Coin toss result: %s"), *ResultString);
    }

    // Notifica il GameMode tramite il delegate
    OnCoinTossCompleted.Broadcast(CoinResult);


}
