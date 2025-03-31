#pragma once

#include "CoreMinimal.h"
#include "GameUnit.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "UnitListWidget.generated.h"


UENUM(BlueprintType)
enum class EOpponentType : uint8
{
	Random  UMETA(DisplayName = "Random"),
	AStar   UMETA(DisplayName = "AStar")
};

UCLASS()
class PA5316005_API UUnitListWidget : public UUserWidget
{
	GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "UnitList")
    void UpdateUnitHealth(bool bIsHuman, EGameUnitType UnitType, float HPPercent);


    UFUNCTION(BlueprintCallable, Category = "UnitList")
    void SetOpponentType(EOpponentType NewOpponent);

protected:
    // --- Human Unit Progress Bars ---
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UProgressBar* PBHumanSniper;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UProgressBar* PBHumanBrawler;

    // --- AI Unit Progress Bars ---
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UProgressBar* PBASniper;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UProgressBar* PBABrawler;

    // --- AI Unit Images ---
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* AISniperImage;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* AIBrawlerImage;

    // --- Textures for AI Unit Images ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitList|Images")
    UTexture2D* RandomSniperTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitList|Images")
    UTexture2D* AStarSniperTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitList|Images")
    UTexture2D* RandomBrawlerTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitList|Images")
    UTexture2D* AStarBrawlerTexture;

    // --- Currently selected opponent type ---
    UPROPERTY(BlueprintReadOnly, Category = "UnitList")
    EOpponentType OpponentType;
};
