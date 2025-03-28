#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AWGameMode.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "MoveHistoryWidget.generated.h"




UCLASS()
class PA5316005_API UMoveHistoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Contenitore principale (per esempio una HorizontalBox) – opzionale se vuoi personalizzare l’aspetto
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HorizontalBox;



	// TextBlock per mostrare il contenuto della mossa
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MoveText;


	// Imposta i dati dell’entry partendo dalla struttura FGameMove
	UFUNCTION(BlueprintCallable)
	void SetData(const FGameMove& Move);


};
