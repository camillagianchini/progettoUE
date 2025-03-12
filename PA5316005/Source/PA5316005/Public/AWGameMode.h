#pragma once

#include "PlayerInterface.h"
#include "GameField.h"
#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Blueprint/UserWidget.h"
#include "AWGameMode.generated.h"

class AActor;

struct FPosition;

USTRUCT(BlueprintType)
struct FMove
{
	GENERATED_BODY()

	// Numero della mossa nell'ordine di esecuzione
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moves")
	int32 MoveNumber;

	// ID dell'unità che ha effettuato la mossa
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moves")
	int32 UnitID;

	// Posizione di partenza
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moves")
	FVector2D Start;

	// Posizione di arrivo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moves")
	FVector2D End;

	// ID dell'unità catturata (se presente, altrimenti -1)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moves")
	int32 CapturedUnitID;



	FMove()
		: MoveNumber(-1), UnitID(-1), Start(FVector2D(-1, -1)), End(FVector2D(-1, -1)), CapturedUnitID(-1)
	{}

	FMove(int32 NewMoveNumber, int32 NewUnitID, FVector2D NewStart, FVector2D NewEnd, int32 NewCapturedUnitID)
		: MoveNumber(NewMoveNumber), UnitID(NewUnitID), Start(NewStart), End(NewEnd), CapturedUnitID(NewCapturedUnitID)
	{}
};

UCLASS()
class PA5316005_API AAWGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// ************ CONSTRUCTORS ************
	AAWGameMode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ************ ATTRIBUTES ************
	// Indica se la partita è terminata
	// Nella sezione degli attributi della AWGameMode
	TMap<int32, AGameUnit*> GameUnitMap;

	bool bIsGameOver;

	// Array degli oggetti che implementano l'interfaccia giocatore (Human e AI)
	
	TArray<IPlayerInterface*> Players;
	TMap<int32, FString> PlayerNames;

	
	int32 CurrentPlayer;


	int32 MoveCounter;

	
	TArray<FMove> Moves;

	// Riferimento al GameField (griglia di gioco)
	UPROPERTY(EditDefaultsOnly, Category = "Game Field")
	TSubclassOf<AGameField> GameFieldClass;

	// Dimensione del campo (es. 25 per una griglia 25x25)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Field")
	int32 FieldSize;

	// Riferimento all'istanza di GameField
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Field")
	AGameField* GField;

	// Indica se lo Sniper del giocatore è stato posizionato
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Placement")
	TMap<int32, bool> bSniperPlaced;

	// Indica se il Brawler del giocatore è stato posizionato
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Placement")
	TMap<int32, bool> bBrawlerPlaced;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> CoinTossWidgetClass;

	// Reference to a Blueprint of Scroll box and Title for the replay
	//UPROPERTY(EditDefaultsOnly)
	//TSubclassOf<class UMovesPanel> PanelWidgetClass;

	// Reference to the panel
	//UPROPERTY(VisibleAnywhere)
	//UMovesPanel* MovesPanel;

	void SetSelectedTile(const FVector2D Position) const;

	int32 GetNextPlayer(int32 Player) const;

	// ************ METODI PRINCIPALI ************
	// Simula il lancio della moneta per decidere chi inizia a posizionare le unità
	void CoinTossForStartingPlayer();
	
	// Inizia il gioco, richiedendo ai giocatori di posizionare le unità
	void ChoosePlayerAndStartGame();

	// Effettua il posizionamento dell'unità di tipo UnitType per il giocatore PlayerID sulla tile TilePosition
	void PlaceUnit(int32 PlayerID, FVector2D TilePosition, EGameUnitType UnitType);


	// Cambia turno, aggiornando CurrentPlayer e gestendo il flusso di gioco
	void TurnNextPlayer();

	// Esegue una mossa data la posizione di destinazione
	// bIsGameMove indica se la mossa viene eseguita nel contesto del gioco (per aggiornare storico, catture, etc.)
	void DoMove(const FVector2D EndPosition, bool bIsGameMove = false);

	// Esegue un attacco sulla tile di destinazione senza muoversi
	void DoAttack(const FVector2D TargetPosition, bool bIsGameMove = false);


	void SetTileMapStatus(const FVector2D Start, const FVector2D End) const;

	// Metodo per verificare la validità di una mossa (se ad esempio la tile è legale per il movimento)
	bool IsIllegalMove() const;


	// Eventuali metodi per verificare le condizioni di vittoria (es. tutte le unità avversarie eliminate)
	bool CheckVictoryCondition() const;

};