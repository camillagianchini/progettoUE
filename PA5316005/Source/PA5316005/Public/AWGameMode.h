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

USTRUCT(BlueprintType)
struct FGameMove
{
    GENERATED_BODY()


    // Identificativo del giocatore (es. "HP" per human, "AI" per l'AI)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerID;

    // Tipo di unità ("S" per Sniper, "B" per Brawler)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString UnitType;

    // Per mossa di movimento: cella di origine e cella di destinazione
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString FromCell;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ToCell;

    // Per mossa di attacco: cella dell’avversario attaccato e danno inflitto
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetCell;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Damage;

    // Flag per distinguere mossa di attacco (true) o di movimento (false)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAttack;

    FGameMove()
        :PlayerID(TEXT("")), UnitType(TEXT("")), FromCell(TEXT("")), ToCell(TEXT("")),
        TargetCell(TEXT("")), Damage(0), bIsAttack(false)
    {
    }
};

class AGameField;
class UMovesPanel;
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
    void CoinTossForStartingPlayer(int32 CoinResult);
    
    void PlaceUnitForCurrentPlayer();
   
    // Funzioni per la fase di battaglia
    void NextTurn();
    bool AllUnitsHaveActed(int32 Player);
    void ResetActionsForPlayer(int32 Player);
    void EndGame();
    int32 GetNextPlayer(int32 Player);



 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UMovesPanel> MovesPanelClass;

    UPROPERTY()
    UMovesPanel* MovesPanel;



    // Indice del giocatore corrente: 0 = Human, 1 = AI
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 CurrentPlayer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn Management")
    int32 StartingPlayer;

    // AAWGameMode.h
    UPROPERTY(BlueprintReadWrite, Category = "CoinToss")
    bool bCoinTossActive = false;

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