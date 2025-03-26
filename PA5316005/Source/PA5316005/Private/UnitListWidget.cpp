#include "UnitListWidget.h"
#include "Components/ProgressBar.h"

void UUnitListWidget::UpdateUnitHealth(bool bIsHuman, EGameUnitType UnitType, float HPPercent)
{
    // Esempio di log (facoltativo)
    UE_LOG(LogTemp, Warning, TEXT("UpdateUnitHealth -> bIsHuman=%d, UnitType=%d, HPPercent=%.2f"),
        (int32)bIsHuman, (int32)UnitType, HPPercent);

    // Aggiorniamo la ProgressBar corretta a seconda che sia Human/AI e Sniper/Brawler
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
