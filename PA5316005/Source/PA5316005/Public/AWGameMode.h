#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AWGameMode.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Placement UMETA(DisplayName = "Placement"),
	Turn      UMETA(DisplayName = "Turn")
};

UCLASS()
class PA5316005_API AAWGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAWGameMode();

	// Called at game start
	virtual void BeginPlay() override;

	//-----------------------------------------
	// RIFERIMENTI AL CAMPO DI GIOCO
	//-----------------------------------------

	// Classe da spawnare per il GameField (la griglia)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grid")
	TSubclassOf<class AGameField> GameFieldClass;

	// Istanza del GameField
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	class AGameField* GField;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SpawnGameField();

	//-----------------------------------------
	// FASE DI POSIZIONAMENTO
	//-----------------------------------------

	// Numero massimo di unit� per giocatore (2 unit�: una per tipo)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement")
	int32 MaxUnitsPerPlayer;

	// Conteggio unit� posizionate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement")
	int32 HumanUnitsPlaced;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement")
	int32 AIUnitsPlaced;

	// Lancio della moneta: se true, Human inizia a posizionare; se false, AI inizia
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement")
	bool bHumanStartsPlacement;

	// Flag per il turno di posizionamento (true = turno Human, false = AI)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement")
	bool bIsHumanPlacementTurn;

	// Inizia la fase di posizionamento
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void StartUnitPlacement();

	// Piazza una unit� su una tile specificata (UnitType: "Sniper" oppure "Brawler")
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void PlaceUnitOnTile(const FVector2D& TilePosition, bool bIsHumanPlayer, FName UnitType);

	//-----------------------------------------
	// FASE DI TURNI DI GIOCO
	//-----------------------------------------

	// Flag per il turno: true = Human, false = AI
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	bool bIsHumanTurn;

	// Inizia il turno del giocatore (il parametro indica se � Human)
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void StartTurn(bool bIsHumanTurnInput);

	// Termina il turno corrente e passa il controllo all'altro giocatore
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void EndTurn();

	//-----------------------------------------
	// CONDIZIONI DI FINE PARTITA
	//-----------------------------------------

	// Array per tenere traccia di tutte le unit� spawnate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	TArray<class AGameUnit*> AllUnits;

	// Registra una nuova unit�
	UFUNCTION(BlueprintCallable, Category = "Game")
	void RegisterUnit(class AGameUnit* NewUnit);

	// Controlla se la partita � finita (vittoria o sconfitta)
	UFUNCTION(BlueprintCallable, Category = "Game")
	void CheckGameEnd();

	//-----------------------------------------
	// METODI DI UTILIT�
	//-----------------------------------------

	// Logga una mossa (da estendere per aggiornare lo storico, widget, ecc.)
	UFUNCTION(BlueprintCallable, Category = "Game")
	void LogMove(const FString& MoveEntry);

	// Evidenzia una tile selezionata (passa la chiamata a GameField)
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SetSelectedTile(const FVector2D& TilePosition);

	//-----------------------------------------
	// Stato del gioco
	//-----------------------------------------

	// Fase attuale del gioco: Placement oppure Turn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Phase")
	EGamePhase CurrentPhase;
};

