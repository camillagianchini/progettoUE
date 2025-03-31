#include "UnitListWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Styling/SlateTypes.h" 


void UUnitListWidget::UpdateUnitHealth(bool bIsHuman, EGameUnitType UnitType, float HPPercent)
{
    //UE_LOG(LogTemp, Warning, TEXT("UpdateUnitHealth -> bIsHuman=%d, UnitType=%d, HPPercent=%.2f"), (int32)bIsHuman, (int32)UnitType, HPPercent);

    if (bIsHuman)
    {
        if (UnitType == EGameUnitType::SNIPER)
        {
            if (PBHumanSniper)
            {
                PBHumanSniper->SetPercent(HPPercent);
            }
        }
        else 
        {
            if (PBHumanBrawler)
            {
                PBHumanBrawler->SetPercent(HPPercent);
            }
        }
    }
    else
    {
        if (UnitType == EGameUnitType::SNIPER)
        {
            if (PBASniper)
            {
                PBASniper->SetPercent(HPPercent);
            }
        }
        else
        {
            if (PBABrawler)
            {
                PBABrawler->SetPercent(HPPercent);
            }
        }
    }
}

// Sets the opponent type and updates the AI unit images accordingly.
void UUnitListWidget::SetOpponentType(EOpponentType NewOpponent)
{
    //UE_LOG(LogTemp, Warning, TEXT("SetOpponentType called with value: %d"), (int32)NewOpponent); OpponentType = NewOpponent;


    if (AISniperImage)
    {
        if (OpponentType == EOpponentType::Random)
        {
           
            AISniperImage->SetBrushFromTexture(RandomSniperTexture, true);
            //UE_LOG(LogTemp, Warning, TEXT("RandomSniperTexture is valid!"));
        }
        else 
        {
            
            AISniperImage->SetBrushFromTexture(AStarSniperTexture, true);
        }
    }

  
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
