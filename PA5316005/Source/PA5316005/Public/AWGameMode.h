#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Sniper.h"
#include "Brawler.h"

#include "AWGameMode.generated.h"


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


    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString UnitType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString FromCell;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ToCell;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetCell;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Damage;

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
class AAStarPlayer;
class UCoinTossWidget;
class UOpponentSelectionWidget;

UCLASS()
class PA5316005_API AAWGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AAWGameMode();

    virtual void BeginPlay() override;

    // ----------------------
    // --- Public Methods ---
    // ----------------------

    // --- Coin Toss functions ---
    UFUNCTION(BlueprintCallable, Category = "Coin Toss")
    void CoinTossForStartingPlayer(int32 CoinResult);
    
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SpawnCoinTossWidget();

    // --- Placement ---
    void PlaceUnitForCurrentPlayer();
   
    // --- Battle ---
    int32 GetNextPlayer(int32 Player);
    void NextTurn();
    bool AllUnitsHaveActed(int32 Player);
    void ResetActionsForPlayer(int32 Player);
    void EndGame();

    // --- Random/AStar ---
    UFUNCTION(BlueprintCallable, Category = "UI")
    void OnOpponentSelected(int32 SelectedOpponent);


    // -------------------------
    // --- Public Properties ---
    // -------------------------
   
   // --- Opponent selection widget ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UOpponentSelectionWidget> OpponentSelectionWidgetClass;

    UPROPERTY()
    UOpponentSelectionWidget* OpponentSelectionWidget;
    
    // --- Moves panel widget ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UMovesPanel> MovesPanelClass;

    UPROPERTY()
    UMovesPanel* MovesPanel;
    
    // --- Unit list widget ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> UnitListWidgetClass;

    UPROPERTY()
    UUserWidget* UnitListWidget;

    // --- Coin toss widget ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<class UCoinTossWidget> CoinTossWidgetClass;

    UPROPERTY()
    UCoinTossWidget* CoinTossWidget;
   
    
   // -----------------------
   // --- Turn ans Player ---
   // -----------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 CurrentPlayer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn Management")
    int32 StartingPlayer;

    UPROPERTY(BlueprintReadWrite, Category = "CoinToss")
    bool bCoinTossActive = false;
 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn Management")
    AGameUnit* SelectedUnit;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EGamePhase CurrentPhase;

    bool bFirstBattleTurn;
    bool bIsAITurnInProgress;


    // --------------------------------
    // --- Field and Map Properties ---
    // --------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FieldSize;


    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TMap<int32, bool> bSniperPlaced;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TMap<int32, bool> bBrawlerPlaced;


    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<AGameField> GameFieldClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    AGameField* GField;


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<AActor*> Players;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TMap<int32, FString> PlayerNames;


    // Game state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsGameOver;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 MoveCounter;


    // --------------------
    // --- Unit Classes ---
    // --------------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Classes|AI")
    TSubclassOf<ASniper> AISniperClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Classes|AI")
    TSubclassOf<ABrawler> AIBrawlerClass;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Classes|Human")
    TSubclassOf<ASniper> HPSniperClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Classes|Human")
    TSubclassOf<ABrawler> HPBrawlerClass;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Classes|AStar")
    TSubclassOf<ASniper> AStarSniperClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Classes|AStar")
    TSubclassOf<ABrawler> AStarBrawlerClass;
};