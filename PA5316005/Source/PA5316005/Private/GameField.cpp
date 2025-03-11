#include "GameField.h"
#include "Kismet/GameplayStatics.h"
#include "GameUnit.h"
#include "Sniper.h"
#include "Brawler.h"
//#include "MovesPanel.h"
#include "AWGameMode.h"
#include "Components/TextRenderComponent.h"

AGameField::AGameField()
{
	PrimaryActorTick.bCanEverTick = false;

	Size = 25;
	TileSize = 150;
	CellPadding = 0;
	NormalizedCellPadding = 0.0f;
	GameUnitScalePercentage = 100;
	ListOfMovesWidgetRef = nullptr;

}

void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	NormalizedCellPadding = FMath::RoundToDouble(((TileSize + CellPadding) / TileSize) * 100) / 100;
}

void AGameField::BeginPlay()
{
	Super::BeginPlay();

	if (ListOfMovesWidgetRef)
	{
		ListOfMovesWidgetRef->AddToViewport(0);
	}

	if (TileArray.Num() == 0)
	{
		GenerateField();
	}

}

void AGameField::SetSelectedTile(FVector2D Position)
{
	SelectedTile = Position;
}

void AGameField::SetLegalMoves(const TArray<FVector2D>& NewLegalMoves)
{
	LegalMovesArray = NewLegalMoves;
}

FVector2D AGameField::GetPosition(const FHitResult& Hit)
{
	return Cast<ATile>(Hit.GetActor())->GetGridPosition();
}

TArray<ATile*>& AGameField::GetTileArray()
{
	return TileArray;
}

TMap<FVector2D, ATile*> AGameField::GetTileMap()
{
	return TileMap;
}

FVector2D AGameField::GetSelectedTile() const
{
	return SelectedTile;
}

TArray<FVector2D> AGameField::GetLegalMoves()
{
	return LegalMovesArray;
}

FVector AGameField::GetRelativePositionByXYPosition(const int32 InX, const int32 InY) const
{
	return TileSize * NormalizedCellPadding * FVector(InX, InY, 0);
}

FVector2D AGameField::GetXYPositionByRelativeLocation(const FVector& Location) const
{
	const double x = Location[0] / (TileSize * NormalizedCellPadding);
	const double y = Location[1] / (TileSize * NormalizedCellPadding);
	return FVector2D(x, y);
}

void AGameField::GenerateField()
{
	for (int32 x = 0; x < Size; x++)
	{
		for (int32 y = 0; y < Size; y++)
		{
			FVector Location = GetRelativePositionByXYPosition(x, y);
			ATile* CurrentTile = GetWorld()->SpawnActor<ATile>(TileClass, Location, FRotator::ZeroRotator);
			const float TileScale = TileSize / 100;
			CurrentTile->SetActorScale3D(FVector(TileScale, TileScale, 0.2));
			CurrentTile->SetGridPosition(x, y);

			TileArray.Add(CurrentTile);
			TileMap.Add(FVector2D(x, y), CurrentTile);

			GenerateLettersAndNumbers(x, y);
		}
	}
}

void AGameField::GenerateLettersAndNumbers(int32 X, int32 Y)
{
	// Verifica che la tile esista nella mappa o nell’array
	if (TileMap.Contains(FVector2D(X, Y)))
	{
		ATile* TheTile = TileMap[FVector2D(X, Y)];
		if (TheTile)
		{
			// Calcolo della lettera (es. A=0, B=1, ecc.)
			// ATTENZIONE: se X può superare 25, servirà una logica diversa (doppia lettera, ecc.)
			char LetterChar = 'A' + X;
			FString LetterString = FString::Printf(TEXT("%c"), LetterChar);

			// Calcolo del numero (es. 1=0, 2=1, ecc.)
			int32 NumberValue = Y + 1;
			FString NumberString = FString::FromInt(NumberValue);

			// Se i componenti di testo esistono, settiamo le scritte
			if (TheTile->TileTextLetter)
			{
				TheTile->TileTextLetter->SetText(FText::FromString(LetterString));
			}

			if (TheTile->TileTextNumber)
			{
				TheTile->TileTextNumber->SetText(FText::FromString(NumberString));
			}

			// Facoltativo: log per debug
			UE_LOG(LogTemp, Log, TEXT("Tile (%d, %d): Letter = %s, Number = %s"),
				X, Y, *LetterString, *NumberString);
		}
	}
}

template<typename T>
void AGameField::GenerateGameUnit(FVector2D Position, int32 Player)
{
	// Verifica che esista una tile nella posizione specificata
	if (!TileMap.Contains(Position))
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessuna tile trovata in posizione %s!"), *Position.ToString());
		return;
	}

	ATile* SelectedTile = TileMap[Position];
	// Controlla se la tile è vuota
	if (!SelectedTile || SelectedTile->GetTileStatus() != ETileStatus::EMPTY)
	{
		UE_LOG(LogTemp, Warning, TEXT("La tile in posizione %s non è vuota!"), *Position.ToString());
		return;
	}

	// Ottieni il riferimento al mondo
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World non trovato!"));
		return;
	}

	// Converti la posizione della griglia in coordinate del mondo
	FVector SpawnLocation = GetRelativePositionByXYPosition(Position.X, Position.Y);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// Spawn dell'unità del tipo specificato
	T* NewUnit = World->SpawnActor<T>(T::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (NewUnit)
	{
		// Imposta il proprietario dell'unità (funzione ipotetica implementata in AGameUnit)
		NewUnit->SetPlayerOwner(Player);

		// Aggiorna lo stato della tile: ora è occupata dall'unità appena spawnata
		SelectedTile->SetTileStatus(Player, ETileStatus::OCCUPIED, NewUnit);

		// Aggiungi l'unità nella mappa delle unità (puoi generare un ID univoco o usare il numero di elementi attuali)
		int32 NewUnitKey = GameUnitMap.Num();
		GameUnitMap.Add(NewUnitKey, NewUnit);

		UE_LOG(LogTemp, Log, TEXT("Unità di tipo %s spawnata in %s per il player %d"), *T::StaticClass()->GetName(), *Position.ToString(), Player);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawn dell'unità di tipo %s fallito in posizione %s"), *T::StaticClass()->GetName(), *Position.ToString());
	}
}


inline bool AGameField::IsValidPosition(const FVector2D Position) const
{
	return Position.X >= 0.f && Position.X < static_cast<float>(Size) &&
		Position.Y >= 0.f && Position.Y < static_cast<float>(Size);
}

void AGameField::SelectTile(const FVector2D Position)
{
	if (IsValidPosition(Position))
	{
		(*TileMap.Find(Position))->SetTileGameStatus(ETileGameStatus::SELECTED);
		SetSelectedTile(Position);
	}
}

void AGameField::ResetGameStatusField()
{
	for (ATile* CurrentTile : TileArray)
	{
		if (CurrentTile->GetTileGameStatus() != ETileGameStatus::FREE)
		{
			CurrentTile->SetTileGameStatus(ETileGameStatus::FREE);
		}
	}

	SetSelectedTile(FVector2D(-1, -1));
}

TArray<FVector2D> AGameField::LegalMoves(FVector2D Position) const
{
	if (!IsValidPosition(Position) || (*TileMap.Find(Position))->GetTileOwner() == NOT_ASSIGNED)
	{
		return TArray<FVector2D>();
	}

	TArray<FVector2D> PossibleMovesOfGameUnit = PossibleMoves(Position);

	AAWGameMode* GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());

	for (int32 index = PossibleMovesOfGameUnit.Num() - 1; index >= 0; --index)
	{
		GameMode->MakeMove(PossibleMovesOfGameUnit[index], false);

		if (GameMode->IsIllegalMove())
		{
			PossibleMovesOfGameUnit.RemoveAt(index);
		}

		GameMode->UndoMove(false);
	}

	return PossibleMovesOfGameUnit;
}

TArray<FVector2D> AGameField::PossibleMoves(FVector2D Position) const
{
	return (*TileMap.Find(Position))->GetGameUnit()->GameUnitLegalMoves();
}

void AGameField::ShowLegalMovesInTheField()
{
	const AAWGameMode* GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());

	for (const FVector2D& Position : LegalMovesArray)
	{
		ATile* CurrentTile = *TileMap.Find(Position);
		
		if (CurrentTile->GetTileOwner() == (GameMode->CurrentPlayer ^ 1))
		{
			CurrentTile->SetTileGameStatus(ETileGameStatus::CAN_ATTACK);
		}
		else
		{
			CurrentTile->SetTileGameStatus(ETileGameStatus::LEGAL_MOVE);
		}

		CurrentTile->SetTileMaterial();
	}
}

void AGameField::ResetField()
{
		// Resetta la tile selezionata e l'array dei movimenti legali
		SelectedTile = FVector2D::ZeroVector;
		LegalMovesArray.Empty();

		// Itera su tutte le tile per resettare il loro stato di gioco
		for (ATile* Tile : TileArray)
		{
			if (Tile)
			{
				// Imposta lo stato della tile come FREE per il gioco e EMPTY per la presenza di unità
				Tile->SetTileGameStatus(ETileGameStatus::FREE);
				Tile->SetTileStatus(NOT_ASSIGNED, ETileStatus::EMPTY, nullptr);

				// Aggiorna il materiale per riflettere il reset (es. colore di default)
				Tile->SetTileMaterial();
			}
		}

		// Distruggi tutte le unità spawnate e svuota la mappa delle unità
		for (auto& Pair : GameUnitMap)
		{
			if (Pair.Value)
			{
				Pair.Value->Destroy();
			}
		}
		GameUnitMap.Empty();

		// Eventuale log di debug
		UE_LOG(LogTemp, Log, TEXT("GameField reset: tutte le tile sono state ripristinate e le unità eliminate."));

		// Notifica ai blueprint (o ad altri sistemi) che il campo è stato resettato
		OnReseTEvent.Broadcast();
}








