#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MoveWidgetEntry.generated.h"

UCLASS()
class PA5316005_API UMoveWidgetEntry : public UUserWidget
{
    GENERATED_BODY()

public:
    // Testo che mostra la mossa effettuata
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* MoveText;

    // Funzione per impostare il contenuto della mossa
    UFUNCTION(BlueprintCallable, Category = "Move Widget")
    void SetMoveText(const FString& MoveString);
};
