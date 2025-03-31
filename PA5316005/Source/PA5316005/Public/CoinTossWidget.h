
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoinTossWidget.generated.h"

class UButton;
class UTextBox;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinTossCompleted, int32, CoinResult);

UCLASS()
class PA5316005_API UCoinTossWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "CoinToss")
	FOnCoinTossCompleted OnCoinTossCompleted;


	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UTextBlock* ResultText;


	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UButton* TossButton;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Coin Toss")
	bool bTossExecuted = false;


	UFUNCTION()
	void OnTossButtonClicked();

protected:

	virtual void NativeConstruct() override;


	void ExecuteCoinToss();
};
