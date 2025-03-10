#include "GameField.h"
#include "Kismet/KismetMathLibrary.h" // Per funzioni random e utility
#include "Math/UnrealMathUtility.h"

AGameField::AGameField()
{
	PrimaryActorTick.bCanEverTick = false;

	// Valori di default
	GridSize = 25;
	ObstaclePercentage = 0.4f;
	bEnsureConnectivity = true;

	TileSize = 120.f;
	CellPadding = 0.0f;
	NextCellPositionMultiplier = 1.0f; // inizializzato correttamente in OnConstruction
}

void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Calcolo del fattore di “spaziatura” orizzontale/verticale tra le celle
	NextCellPositionMultiplier = (TileSize + TileSize * CellPadding) / TileSize;
}

void AGameField::BeginPlay()
{
	Super::BeginPlay();

	GenerateField();
}

//------------------------------------------------------------------------------
// GENERAZIONE E DISTRUZIONE DELLA GRIGLIA
//------------------------------------------------------------------------------

void AGameField::GenerateField()
{
	// Prima di generare, puliamo eventuali celle esistenti
	ClearField();

	if (!TileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TileClass non è impostata in AGameField!"));
		return;
	}

	// Crea tutte le celle e memorizzale in TileArray e TileMap
	for (int32 X = 0; X < GridSize; X++)
	{
		for (int32 Y = 0; Y < GridSize; Y++)
		{
			const FVector SpawnLocation = GetRelativeLocationByXYPosition(X, Y);
			const FRotator SpawnRotation = FRotator::ZeroRotator;

			ATile* SpawnedTile = GetWorld()->SpawnActor<ATile>(
				TileClass,
				SpawnLocation,
				SpawnRotation
			);

			if (SpawnedTile)
			{
				// Scala la tile per visualizzarla correttamente
				const float TileScale = TileSize / 100.f;
				const float ZScaling = 0.2f;
				SpawnedTile->SetActorScale3D(FVector(TileScale, TileScale, ZScaling));

				// Imposta la posizione interna (X, Y)
				SpawnedTile->SetGridPosition(X, Y);

				// Di default, la cella è vuota (EMPTY)
				SpawnedTile->SetTileStatus(-1, ETileStatus::EMPTY);

				// Aggiunge i riferimenti a TileArray e TileMap
				TileArray.Add(SpawnedTile);
				TileMap.Add(FVector2D(X, Y), SpawnedTile);
			}
		}
	}

	// Genera gli ostacoli in base alla percentuale
	GenerateObstacles();

	// Se richiesto, controlla la connettività
	if (bEnsureConnectivity)
	{
		if (!EnsureConnectivity())
		{
			UE_LOG(LogTemp, Warning, TEXT("La mappa generata non era connessa, è stata rigenerata."));
		}
	}
}

void AGameField::ClearField()
{
	// Distrugge tutte le celle e pulisce gli array/mappe
	for (ATile* Tile : TileArray)
	{
		if (Tile && Tile->IsValidLowLevel())
		{
			Tile->Destroy();
		}
	}
	TileArray.Empty();
	TileMap.Empty();
}

//------------------------------------------------------------------------------
// OSTACOLI E CONNETTIVITÀ
//------------------------------------------------------------------------------

void AGameField::GenerateObstacles()
{
	// Numero di celle totali
	const int32 TotalCells = GridSize * GridSize;
	// Numero di ostacoli da generare
	const int32 NumObstacles = FMath::FloorToInt(TotalCells * ObstaclePercentage);

	// Genera una lista di tutte le coordinate disponibili
	TArray<FVector2D> AllCoords;
	AllCoords.Reserve(TotalCells);

	for (int32 X = 0; X < GridSize; X++)
	{
		for (int32 Y = 0; Y < GridSize; Y++)
		{
			AllCoords.Add(FVector2D(X, Y));
		}
	}

	// Mescola la lista
	// Fisher-Yates Shuffle
	for (int32 i = AllCoords.Num() - 1; i > 0; --i)
	{
		// Estrae un indice casuale tra 0 e i
		int32 j = FMath::RandRange(0, i);
		// Scambia l’elemento in posizione i con quello in posizione j
		AllCoords.Swap(i, j);
	}


	// Trasforma in ostacoli le prime NumObstacles celle
	int32 PlacedObstacles = 0;
	for (int32 i = 0; i < AllCoords.Num() && PlacedObstacles < NumObstacles; i++)
	{
		FVector2D Coord = AllCoords[i];
		ATile* Tile = TileMap[Coord];
		if (Tile && Tile->GetTileStatus() == ETileStatus::EMPTY)
		{
			Tile->SetTileStatus(-1, ETileStatus::OBSTACLE);
			PlacedObstacles++;
		}
	}
}

bool AGameField::EnsureConnectivity()
{
	// Controlla se la mappa è connessa
	if (IsMapFullyReachable())
	{
		return true;
	}

	// Se non lo è, rigenera finché non otteniamo una mappa connessa
	const int32 MaxAttempts = 20;
	for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
	{
		ClearField();
		GenerateField();

		if (IsMapFullyReachable())
		{
			return true;
		}
	}

	// Se anche dopo diversi tentativi non ci riusciamo, restituiamo false
	return false;
}

bool AGameField::IsMapFullyReachable() const
{
	// 1) Trova una cella EMPTY di partenza
	FVector2D StartCoord(-1, -1);

	for (auto& Elem : TileMap)
	{
		if (Elem.Value && Elem.Value->GetTileStatus() == ETileStatus::EMPTY)
		{
			StartCoord = Elem.Key;
			break;
		}
	}

	// Se non abbiamo trovato neanche una cella libera, consideriamo la mappa triviale
	if (StartCoord.X < 0)
	{
		return true;
	}

	// 2) Esegui una BFS/DFS per trovare tutte le celle EMPTY raggiungibili
	TSet<FVector2D> Visited;
	TQueue<FVector2D> Frontier;
	Frontier.Enqueue(StartCoord);
	Visited.Add(StartCoord);

	while (!Frontier.IsEmpty())
	{
		FVector2D Current;
		Frontier.Dequeue(Current);

		TArray<FVector2D> Neighbors = GetNeighbors(Current);
		for (const FVector2D& N : Neighbors)
		{
			if (!Visited.Contains(N))
			{
				Visited.Add(N);
				Frontier.Enqueue(N);
			}
		}
	}

	// 3) Se il numero di celle visitate è pari al numero di celle EMPTY globali, la mappa è connessa
	int32 NumEmptyCells = 0;
	for (auto& Elem : TileMap)
	{
		if (Elem.Value && Elem.Value->GetTileStatus() == ETileStatus::EMPTY)
		{
			NumEmptyCells++;
		}
	}

	return (Visited.Num() == NumEmptyCells);
}

TArray<FVector2D> AGameField::GetNeighbors(const FVector2D& CurrentCoord) const
{
	TArray<FVector2D> Result;

	// Movimenti ortogonali (su, giù, sinistra, destra)
	static const TArray<FVector2D> Directions = {
		FVector2D(1, 0),
		FVector2D(-1, 0),
		FVector2D(0, 1),
		FVector2D(0, -1),
	};

	for (const FVector2D& Dir : Directions)
	{
		FVector2D NextCoord = CurrentCoord + Dir;
		if (IsValidCoordinate(NextCoord))
		{
			ATile* NextTile = TileMap[NextCoord];
			if (NextTile && NextTile->GetTileStatus() == ETileStatus::EMPTY)
			{
				Result.Add(NextCoord);
			}
		}
	}

	return Result;
}

//------------------------------------------------------------------------------
// UTILITY
//------------------------------------------------------------------------------

FVector2D AGameField::GetPosition(const FHitResult& Hit) const
{
	ATile* HitTile = Cast<ATile>(Hit.GetActor());
	if (HitTile)
	{
		return HitTile->GetGridPosition();
	}
	return FVector2D(-1, -1);
}

FVector AGameField::GetRelativeLocationByXYPosition(int32 InX, int32 InY) const
{
	// Moltiplichiamo la TileSize per NextCellPositionMultiplier
	return TileSize * NextCellPositionMultiplier * FVector(InX, InY, 0);
}

FVector2D AGameField::GetXYPositionByRelativeLocation(const FVector& Location) const
{
	// Facciamo l’inverso di quanto fatto sopra
	const double XPos = Location.X / (TileSize * NextCellPositionMultiplier);
	const double YPos = Location.Y / (TileSize * NextCellPositionMultiplier);
	return FVector2D(XPos, YPos);
}

bool AGameField::IsValidCoordinate(const FVector2D& Coord) const
{
	return (Coord.X >= 0 && Coord.X < GridSize && Coord.Y >= 0 && Coord.Y < GridSize);
}


