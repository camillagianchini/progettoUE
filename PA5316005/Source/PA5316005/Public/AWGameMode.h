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

    // Funzioni per la fase di posizionamento
    void CoinTossForStartingPlayer();
    void ChoosePlayerAndStartGame();
    void PlaceUnitForCurrentPlayer();

    // Funzioni per la fase di battaglia
    void NextTurn();
    bool TutteLeUnitaHannoAgito(int32 Player);
    bool CondizioniDiVittoria();
    void ResetActionsForPlayer(int32 Player);
    void EndGame();

    UFUNCTION(BlueprintCallable)
    bool DoMove(FVector2D Destination, bool bTestOnly);

    // Se il tuo codice chiama TurnNextPlayer(), dichiari la funzione:
    UFUNCTION(BlueprintCallable)
    void TurnNextPlayer();

    // Se chiama SetSelectedTile(FVector2D), dichiari la funzione:
    UFUNCTION(BlueprintCallable)
    void SetSelectedTile(FVector2D Position);

    // Se chiama PlaceUnit(...) con certi parametri, dichiara la funzione:
    UFUNCTION(BlueprintCallable)
    void PlaceUnit(int32 Player, FVector2D Position /* o altro se necessario */);

    // Se chiama DoAttack(...) con certi parametri, dichiara la funzione:
    UFUNCTION(BlueprintCallable)
    void DoAttack(FVector2D AttackerPosition, FVector2D TargetPosition);


    // Variabili di stato

    // Indice del giocatore corrente: 0 = Human, 1 = AI
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 CurrentPlayer;

    // Dimensione del campo (griglia NxN)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 FieldSize;

    // Mappe per il posizionamento delle unità per ciascun giocatore
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
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

  

    // Fase corrente del gioco: Placement oppure Battle
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EGamePhase CurrentPhase;
};
