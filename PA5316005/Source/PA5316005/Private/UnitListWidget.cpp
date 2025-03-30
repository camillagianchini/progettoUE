#include "UnitListWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

#include "Styling/SlateTypes.h"

void UUnitListWidget::UpdateUnitHealth(bool bIsHuman, EGameUnitType UnitType, float HPPercent)
{
    UE_LOG(LogTemp, Warning, TEXT("UpdateUnitHealth -> bIsHuman=%d, UnitType=%d, HPPercent=%.2f"),
        (int32)bIsHuman, (int32)UnitType, HPPercent);

    if (bIsHuman)
    {
        if (UnitType == EGameUnitType::SNIPER)
        {
            if (PBHumanSniper)
            {
                PBHumanSniper->SetPercent(HPPercent);
            }
        }
        else // BRAWLER
        {
            if (PBHumanBrawler)
            {
                PBHumanBrawler->SetPercent(HPPercent);
            }
        }
    }
    else // AI
    {
        if (UnitType == EGameUnitType::SNIPER)
        {
            if (PBASniper)
            {
                PBASniper->SetPercent(HPPercent);
            }
        }
        else // BRAWLER
        {
            if (PBABrawler)
            {
                PBABrawler->SetPercent(HPPercent);
            }
        }
    }
}

void UUnitListWidget::SetOpponentType(EOpponentType NewOpponent)
{
    UE_LOG(LogTemp, Warning, TEXT("SetOpponentType called with value: %d"), (int32)NewOpponent);
    OpponentType = NewOpponent;

    // Esempio: se ho un AISniperImage e voglio settare la texture 
    if (AISniperImage)
    {
        if (OpponentType == EOpponentType::Random)
        {
            AISniperImage->SetBrushFromTexture(RandomSniperTexture, /* bMatchSize = */ true);
            UE_LOG(LogTemp, Warning, TEXT("RandomSniperTexture is valid!"));
        }
        else // EOpponentType::AStar
        {
            AISniperImage->SetBrushFromTexture(AStarSniperTexture, true);
        }
    }

    // Idem per AIBrawlerImage
    if (AIBrawlerImage)
    {
        if (OpponentType == EOpponentType::Random)
        {
            AIBrawlerImage->SetBrushFromTexture(RandomBrawlerTexture, true);
        }
        else
        {
            AIBrawlerImage->SetBrushFromTexture(AStarBrawlerTexture, true);
        }
    }
}
