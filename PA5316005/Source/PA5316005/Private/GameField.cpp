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
	NormalizedCellPadding = 1;
	GameUnitScalePercentage = 100;
	ListOfMovesWidgetRef = nullptr;

}

TArray<FVector2D> AGameField::GetValidMoves(AGameUnit* Unit) const
{
	TArray<FVector2D> ValidMoves;

	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetValidMoves: Unit� nulla"));
		return ValidMoves;
	}

	// Calcola le mosse legali base per l'unit�
	TArray<FVector2D> LegalMoves = Unit->CalculateLegalMoves();

	// Filtra ogni mossa: controlla se � dentro la griglia e se la tile � libera
	for (const FVector2D& MovePos : LegalMoves)
	{
		// Controlla se la posizione � valida all'interno della griglia
		if (!IsValidPosition(MovePos))
		{
			continue;
		}

		// Recupera la tile corrispondente
		ATile* Tile = nullptr;
		if (TileMap.Contains(MovePos))
		{
			Tile = TileMap[MovePos];
		}

		// Se la tile esiste e risulta libera, la aggiungiamo
		if (Tile && Tile->GetTileStatus() == ETileStatus::EMPTY)
		{
			ValidMoves.Add(MovePos);
		}
	}

	return ValidMoves;
}


void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	NormalizedCellPadding = FMath::RoundToDouble(((TileSize + CellPadding) / TileSize) * 100) / 100;
}

void AGameField::BeginPlay()
{
	Super::BeginPlay();

	//if (ListOfMovesWidgetRef)
	//{
		//ListOfMovesWidgetRef->AddToViewport(0);
	//}

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
	// Calcola la posizione relativa rispetto all'origine del GameField
	FVector RelativeLocation = Location - GetActorLocation();

	// Dividi per la dimensione del tile per ottenere la griglia
	double x = RelativeLocation.X / (TileSize * NormalizedCellPadding);
	double y = RelativeLocation.Y / (TileSize * NormalizedCellPadding);

	return FVector2D(FMath::RoundToFloat(x), FMath::RoundToFloat(y));
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
	// Verifica che la tile esista nella mappa o nell�array
	if (TileMap.Contains(FVector2D(X, Y)))
	{
		ATile* TheTile = TileMap[FVector2D(X, Y)];
		if (TheTile)
		{
			// Calcolo della lettera (es. A=0, B=1, ecc.)
			// ATTENZIONE: se X pu� superare 25, servir� una logica diversa (doppia lettera, ecc.)
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

	ATile* LocalSelectedTile = TileMap[Position];
	// Controlla se la tile � vuota
	if (!LocalSelectedTile || LocalSelectedTile->GetTileStatus() != ETileStatus::EMPTY)
	{
		UE_LOG(LogTemp, Warning, TEXT("La tile in posizione %s non � vuota!"), *Position.ToString());
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

	// Spawn dell'unit� del tipo specificato
	T* NewUnit = World->SpawnActor<T>(T::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (NewUnit)
	{
		NewUnit->SetPlayerOwner(Player);

		// Aggiorna lo stato della tile: ora � occupata dall'unit� appena spawnata
		LocalSelectedTile->SetTileStatus(Player, ETileStatus::OCCUPIED, NewUnit);

		// Aggiungi l'unit� nella mappa delle unit�
		int32 NewUnitKey = GameUnitMap.Num();
		GameUnitMap.Add(NewUnitKey, NewUnit);

		UE_LOG(LogTemp, Log, TEXT("Unit� di tipo %s spawnata in %s per il player %d"), *T::StaticClass()->GetName(), *Position.ToString(), Player);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawn dell'unit� di tipo %s fallito in posizione %s"), *T::StaticClass()->GetName(), *Position.ToString());
	}
}



template void AGameField::GenerateGameUnit<ASniper>(FVector2D Position, int32 Player);
template void AGameField::GenerateGameUnit<ABrawler>(FVector2D Position, int32 Player);


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

TArray<FVector2D> AGameField::PossibleMoves(FVector2D Position) const
{
	TArray<FVector2D> Moves;

	// Cerca la tile nella mappa
	if (TileMap.Contains(Position))
	{
		ATile* Tile = TileMap[Position];
		if (Tile && Tile->GetGameUnit())
		{
			// Supponiamo che la GameUnit abbia un metodo CalculateLegalMoves()
			Moves = Tile->GetGameUnit()->CalculateLegalMoves();
		}
	}

	return Moves;
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
		GameMode->DoMove(PossibleMovesOfGameUnit[index], false);

	}

	return PossibleMovesOfGameUnit;
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
				// Imposta lo stato della tile come FREE per il gioco e EMPTY per la presenza di unit�
				Tile->SetTileGameStatus(ETileGameStatus::FREE);
				Tile->SetTileStatus(NOT_ASSIGNED, ETileStatus::EMPTY, nullptr);

				// Aggiorna il materiale per riflettere il reset (es. colore di default)
				Tile->SetTileMaterial();
			}
		}

		// Distruggi tutte le unit� spawnate e svuota la mappa delle unit�
		for (auto& Pair : GameUnitMap)
		{
			if (Pair.Value)
			{
				Pair.Value->Destroy();
			}
		}
		GameUnitMap.Empty();

		// Eventuale log di debug
		UE_LOG(LogTemp, Log, TEXT("GameField reset: tutte le tile sono state ripristinate e le unit� eliminate."));

		// Notifica ai blueprint (o ad altri sistemi) che il campo � stato resettato
		OnResetEvent.Broadcast();
}

ATile* AGameField::GetRandomFreeTile() const
{
	TArray<ATile*> FreeTiles;
	for (ATile* Tile : TileArray)
	{
		if (Tile && Tile->GetTileStatus() == ETileStatus::EMPTY)
		{
			FreeTiles.Add(Tile);
		}
	}

	if (FreeTiles.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, FreeTiles.Num() - 1);
		return FreeTiles[RandomIndex];
	}

	return nullptr;
}


void AGameField::MoveUnit(AGameUnit* Unit, const FVector2D& NewPos)
{
	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveUnit: Unit� nulla!"));
		return;
	}

	// Ottieni la posizione di partenza
	FVector2D OldPos = Unit->GetGridPosition();

	// Verifica che la tile di partenza esista ed � occupata da questa unit�
	if (!TileMap.Contains(OldPos))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveUnit: la tile di partenza %s non esiste nella mappa."), *OldPos.ToString());
		return;
	}

	ATile* OldTile = TileMap[OldPos];


	// Verifica che la tile di destinazione sia valida e libera
	if (!IsValidPosition(NewPos) || !TileMap.Contains(NewPos))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveUnit: la tile di destinazione %s non � valida."), *NewPos.ToString());
		return;
	}

	ATile* NewTile = TileMap[NewPos];
	if (NewTile->GetTileStatus() != ETileStatus::EMPTY)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveUnit: la tile di destinazione %s non � libera."), *NewPos.ToString());
		return;
	}

	// Aggiorna la tile di partenza (diventa EMPTY)
	OldTile->SetTileStatus(NOT_ASSIGNED, ETileStatus::EMPTY, nullptr);

	// Aggiorna la tile di destinazione (diventa OCCUPIED)
	NewTile->SetTileStatus(Unit->GetPlayerOwner(), ETileStatus::OCCUPIED, Unit);

	// Aggiorna la posizione interna dell'unit�
	Unit->SetGridPosition(NewPos.X, NewPos.Y);


	// Se vuoi spostare anche visivamente la mesh, puoi calcolare la nuova posizione in world space
	// e chiamare Unit->SetActorLocation(...) o fare un Lerp. Qui facciamo un teletrasporto veloce:
	FVector NewWorldPos = GetRelativePositionByXYPosition(NewPos.X, NewPos.Y);
	Unit->SetActorLocation(FVector(NewWorldPos.X, NewWorldPos.Y, Unit->GetActorLocation().Z));

	UE_LOG(LogTemp, Log, TEXT("MoveUnit: Unit� ID %d mossa da %s a %s"),
		Unit->GetGameUnitID(), *OldPos.ToString(), *NewPos.ToString());
}

void AGameField::AttackUnit(AGameUnit* Attacker, const FVector2D& TargetPos)
{
	if (!Attacker)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Attacker nullo!"));
		return;
	}

	// Verifica che la tile di destinazione esista
	if (!IsValidPosition(TargetPos) || !TileMap.Contains(TargetPos))
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Posizione di destinazione %s non valida."), *TargetPos.ToString());
		return;
	}

	ATile* TargetTile = TileMap[TargetPos];
	if (!TargetTile || TargetTile->GetTileStatus() != ETileStatus::OCCUPIED)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Nessuna unit� da attaccare sulla tile %s."), *TargetPos.ToString());
		return;
	}

	AGameUnit* Defender = TargetTile->GetGameUnit();
	if (!Defender)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Tile occupata ma unit� nulla?"));
		return;
	}

	// Verifica che sia un'unit� nemica
	if (Defender->GetPlayerOwner() == Attacker->GetPlayerOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Non puoi attaccare un'unit� tua!"));
		return;
	}

	// Calcolo del danno
	// Esempio: valore random tra DamageMin e DamageMax dell'Attacker
	int32 Damage = FMath::RandRange(Attacker->GetDamageMin(), Attacker->GetDamageMax());
	UE_LOG(LogTemp, Log, TEXT("AttackUnit: L'unit� ID %d infligge %d danni a unit� ID %d"),
		Attacker->GetGameUnitID(), Damage, Defender->GetGameUnitID());

	// Applica il danno
	Defender->TakeDamageUnit(Damage);

	// Se il Defender � morto, rimuovilo dalla tile e distruggilo
	if (Defender->IsDead())
	{
		UE_LOG(LogTemp, Log, TEXT("AttackUnit: L'unit� ID %d � morta."), Defender->GetGameUnitID());
		// Aggiorna la tile
		TargetTile->SetTileStatus(NOT_ASSIGNED, ETileStatus::EMPTY, nullptr);
		// Rimuovi anche dalla mappa (se la usi)
		// Trova la chiave e rimuovi Defender da GameUnitMap
		for (auto It = GameUnitMap.CreateIterator(); It; ++It)
		{
			if (It.Value() == Defender)
			{
				It.RemoveCurrent();
				break;
			}
		}
		Defender->Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("AttackUnit: L'unit� ID %d sopravvive con %d HP"),
			Defender->GetGameUnitID(), Defender->GetHitPoints());
	}
}





