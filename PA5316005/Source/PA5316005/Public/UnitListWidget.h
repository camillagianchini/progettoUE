// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameUnit.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "UnitListWidget.generated.h"

/**
 * 
 */
UCLASS()
class PA5316005_API UUnitListWidget : public UUserWidget
{
	GENERATED_BODY()

public:

    // Funzione che chiamerai dal C++ (TakeDamageUnit) per aggiornare le barre
    UFUNCTION(BlueprintCallable, Category = "UnitList")
    void UpdateUnitHealth(bool bIsHuman, EGameUnitType UnitType, float HPPercent);

    // Qui dichiari i riferimenti ai tuoi Progress Bar (uno per ogni combinazione).
    // Questi campi vanno poi "bindati" nel Blueprint (vedi sotto).

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UProgressBar* PBHumanSniper;    // Barra HP Sniper umano

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UProgressBar* PBHumanBrawler;   // Barra HP Brawler umano

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UProgressBar* PBASniper;        // Barra HP Sniper AI

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UProgressBar* PBABrawler;       // Barra HP Brawler AI
	
};
