#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoinTossWidget.generated.h"

/**
 * Widget per mostrare il risultato del lancio della moneta
 */
UCLASS()
class PA5316005_API UCoinTossWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Funzione per aggiornare il testo mostrato (da chiamare dal GameMode o dal controller)
	UFUNCTION(BlueprintCallable, Category = "Coin Toss")
	void SetResultText(const FText& NewText);

protected:
	// Bind del TextBlock presente nel widget Blueprint (deve avere lo stesso nome)
	// Assicurati di aver creato un widget Blueprint basato su questa classe e di avere un TextBlock con il nome "ResultTextBlock"
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ResultTextBlock;
};
