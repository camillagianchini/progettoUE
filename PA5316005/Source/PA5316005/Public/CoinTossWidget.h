#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoinTossWidget.generated.h"

UCLASS()
class PA5316005_API UCoinTossWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // TextBlock per mostrare il risultato. 
    // (Lo collegherai in Blueprint con “BindWidget”)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* CoinTossResultText;

    // Funzione C++ richiamabile da GameMode per aggiornare il testo
    UFUNCTION(BlueprintCallable, Category = "CoinToss")
    void SetCoinTossResult(const FString& ResultText);
};
