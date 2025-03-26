#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Sniper.h"
#include "Brawler.h"

#include "AWGameMode.generated.h"


// Definizione dell'enum per le fasi del gioco
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Placement UMETA(DisplayName = "Placement"),
    Battle    UMETA(DisplayName = "Battle")
};

class AGameField;
class AHumanPlayer;
class ARandomPlayer;
class UCoinTossWidget;

UCLASS()
class PA5316005_API AAWGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AAWGameMode();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Coin Toss")
    void CoinTossForStartingPlayer();
    
    void PlaceUnitForCurrentPlayer();

    // Funzioni per la fase di battaglia
    void NextTurn();
    bool AllUnitsHaveActed(int32 Player);
    void ResetActionsForPlayer(int32 Player);
    void EndGame();
    int32 GetNextPlayer(int32 Player);


    // Variabili di stato

    // Indice del giocatore corrente: 0 = Human, 1 = AI
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 CurrentPlayer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn Management")
    int32 StartingPlayer;

    // AAWGameMode.h

// 1) Una variabile per la classe del widget
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> UnitListWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<class UCoinTossWidget> CoinTossWidgetClass;

    UPROPERTY()
    UCoinTossWidget* CoinTossWidget;
    // 2) Una variabile per l'istanza creata a runtime
    UPROPERTY()
    UUserWidget* UnitListWidget;

    bool bFirstBattleTurn;
    bool bIsAITurnInProgress;
    // Dimensione del campo (griglia NxN)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FieldSize;

    // Mappe per il posizionamento delle unità per ciascun giocatore
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TMap<int32, bool> bSniperPlaced;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TMap<int32, bool> bBrawlerPlaced;

    // Classe del GameField da spawnare (Blueprint)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<AGameField> GameFieldClass;

    // Istanza del GameField spawnato
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    AGameField* GField;

    // Array dei giocatori (es. HumanPlayer e RandomPlayer)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<AActor*> Players;

    // Mappa dei nomi dei giocatori
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TMap<int32, FString> PlayerNames;

    // Stato del gioco
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsGameOver;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 MoveCounter;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Classes|AI")
    TSubclassOf<ASniper> AISniperClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Classes|AI")
    TSubclassOf<ABrawler> AIBrawlerClass;

    // Per il giocatore umano
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Classes|Human")
    TSubclassOf<ASniper> HPSniperClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Classes|Human")
    TSubclassOf<ABrawler> HPBrawlerClass;

    // AAWGameMode.h
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn Management")
    AGameUnit* SelectedUnit;


    // Fase corrente del gioco: Placement oppure Battle
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EGamePhase CurrentPhase;
};