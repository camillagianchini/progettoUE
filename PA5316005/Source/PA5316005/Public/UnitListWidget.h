#pragma once

#include "CoreMinimal.h"
#include "GameUnit.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "UnitListWidget.generated.h"

// Enum per il tipo di avversario
UENUM(BlueprintType)
enum class EOpponentType : uint8
{
	Random  UMETA(DisplayName = "Random"),
	AStar   UMETA(DisplayName = "AStar")
};

/**
 * Widget per visualizzare la salute delle unità
 */
UCLASS()
class PA5316005_API UUnitListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Funzione per aggiornare la salute della progress bar in base alle unità
	UFUNCTION(BlueprintCallable, Category = "UnitList")
	void UpdateUnitHealth(bool bIsHuman, EGameUnitType UnitType, float HPPercent);

	// Funzione per impostare il tipo di avversario e aggiornare le immagini corrispondenti
	UFUNCTION(BlueprintCallable, Category = "UnitList")
	void SetOpponentType(EOpponentType NewOpponent);

protected:
	// Progress Bar per le unità umane
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* PBHumanSniper;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* PBHumanBrawler;

	// Progress Bar per le unità AI
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* PBASniper;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* PBABrawler;

	// Immagini per rappresentare le unità AI: cambiano in base all'avversario scelto
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* AISniperImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* AIBrawlerImage;

	// Texture da assegnare alle immagini, in base al tipo di avversario
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitList|Images")
	UTexture2D* RandomSniperTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitList|Images")
	UTexture2D* AStarSniperTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitList|Images")
	UTexture2D* RandomBrawlerTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitList|Images")
	UTexture2D* AStarBrawlerTexture;

	// Variabile che memorizza il tipo di avversario selezionato
	UPROPERTY(BlueprintReadOnly, Category = "UnitList")
	EOpponentType OpponentType;
};
