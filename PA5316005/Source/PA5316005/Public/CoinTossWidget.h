// CoinTossWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoinTossWidget.generated.h"

class UButton;
class UTextBox;

// Dichiarazione del delegate per notificare il completamento del coin toss
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinTossCompleted, int32, CoinResult);

UCLASS()
class PA5316005_API UCoinTossWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "CoinToss")
	FOnCoinTossCompleted OnCoinTossCompleted;

	// TextBlock per mostrare l'esito
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UTextBlock* ResultText;

	// **Bottone** su cui cliccare per lanciare la moneta
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UButton* TossButton;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Coin Toss")
	bool bTossExecuted = false;


	// Funzione chiamata quando premiamo il bottone
	UFUNCTION()
	void OnTossButtonClicked();

protected:
	virtual void NativeConstruct() override;

	// Esegue effettivamente il coin toss
	void ExecuteCoinToss();
};
