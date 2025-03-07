#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MoveWidgetEntry.h"
#include "MovesPanel.generated.h"

UCLASS()
class PA5316005_API UMovesPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    // Lista delle mosse effettuate
    UPROPERTY(meta = (BindWidget))
    class UScrollBox* MovesList;

    // Classe di riferimento per i widget delle mosse
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> MoveEntryClass;

    // Aggiunge una mossa alla UI
    UFUNCTION(BlueprintCallable)
    void AddMove(const FString& MoveString);
};
