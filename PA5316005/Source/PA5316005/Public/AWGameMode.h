#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlayerInterface.h"
#include "AWGameMode.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    PreGame UMETA(DisplayName = "PreGame"),      // Lancio di moneta e posizionamento unità
    InGame  UMETA(DisplayName = "InGame"),       // Partita vera e propria
    EndGame UMETA(DisplayName = "EndGame")       // Fine partita
};

UCLASS()
class PA5316005_API AAWGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AAWGameMode();

    virtual void BeginPlay() override;

    // Variabile per tracciare la fase corrente
    UPROPERTY(BlueprintReadOnly, Category = "GameFlow")
    EGamePhase CurrentPhase;

    UPROPERTY()
    TScriptInterface<IPlayerInterface> HumanPlayer;

    UPROPERTY()
    TScriptInterface<IPlayerInterface> AIPlayer;


    // Coin toss result
    bool bHumanStarts;

    // Numero di unità da posizionare ancora per ciascun player
    int32 HumanUnitsToPlace;
    int32 AIUnitsToPlace;

    // Funzioni principali
    void StartPreGame();
    void DoCoinToss();
    void StartPlacingUnits();

    // Quando un player piazza un’unità
    UFUNCTION()
    void OnUnitPlaced(bool bIsHuman);

    // Passaggio da posizionamento a inizio partita
    UFUNCTION()
    void StartGame();

    // Funzione per la Fine Partita
    UFUNCTION()
    void EndMatch(bool bHumanWon);
};

