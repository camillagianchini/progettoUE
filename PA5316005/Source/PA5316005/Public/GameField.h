#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.h"
#include "GameField.generated.h"

UCLASS()
class PA5316005_API AGameField : public AActor
{
	GENERATED_BODY()

public:
	// Costruttore
	AGameField();

	// Chiamato quando l’istanza della classe è piazzata in editor o spawnata a runtime
	virtual void OnConstruction(const FTransform& Transform) override;

	// Chiamato all’avvio del gioco (dopo BeginPlay)
	virtual void BeginPlay() override;

	//-----------------------------------------
	// CONFIGURAZIONE DELLA GRIGLIA
	//-----------------------------------------

	// Dimensione della griglia (Default 25)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Settings")
	int32 GridSize;

	// Percentuale di ostacoli da generare in automatico (Default 0.1 = 10%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Settings", meta = (ClampMin = "0.0", ClampMax = "0.9"))
	float ObstaclePercentage;

	// Se true, forza la connettività di tutte le celle libere
	// (se la generazione casuale crea “isole” non connesse, rigenera gli ostacoli)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Settings")
	bool bEnsureConnectivity;

	//-----------------------------------------
	// TILES
	//-----------------------------------------

	// Riferimento alla classe Tile da spawnare
	UPROPERTY(EditDefaultsOnly, Category = "Grid Settings")
	TSubclassOf<ATile> TileClass;

	// Vettore di Tile
	UPROPERTY(Transient)
	TArray<ATile*> TileArray;

	// Mappa che associa una posizione 2D alla Tile corrispondente
	UPROPERTY(Transient)
	TMap<FVector2D, ATile*> TileMap;

	// Dimensione di una singola cella (in Unreal Units)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Settings")
	float TileSize;

	// Percentuale di “padding” tra una cella e l’altra
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Settings")
	float CellPadding;

	// Usato per calcolare la distanza effettiva tra due celle adiacenti
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Settings")
	float NextCellPositionMultiplier;

	//-----------------------------------------
	// METODI PUBBLICI
	//-----------------------------------------

	// Genera l’intera griglia (viene chiamato in BeginPlay, ma puoi richiamarlo se vuoi rigenerare la mappa)
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void GenerateField();

	// Distrugge tutte le tile esistenti
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void ClearField();

	// Genera casualmente ostacoli nella griglia in base a ObstaclePercentage
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void GenerateObstacles();

	// Se bEnsureConnectivity = true, controlla che tutte le celle libere siano connesse fra loro
	// e rigenera gli ostacoli se non lo sono.
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool EnsureConnectivity();

	// Ritorna la tile su cui si è cliccato, estraendo la posizione dalla HitResult
	FVector2D GetPosition(const FHitResult& Hit) const;

	// Converte coordinate (x, y) in uno spawn location 3D
	FVector GetRelativeLocationByXYPosition(int32 InX, int32 InY) const;

	// Converte una location 3D in coordinate (x, y) (può servire per calcoli di pathfinding o debug)
	FVector2D GetXYPositionByRelativeLocation(const FVector& Location) const;

	// Verifica se (x, y) è una posizione valida nella griglia
	bool IsValidCoordinate(const FVector2D& Coord) const;

protected:

	// Esegue un controllo BFS/DFS per verificare se tutte le celle libere sono connesse
	bool IsMapFullyReachable() const;

	// Ritorna le celle adiacenti (su/giù/destra/sinistra) valide e libere
	TArray<FVector2D> GetNeighbors(const FVector2D& CurrentCoord) const;
};
