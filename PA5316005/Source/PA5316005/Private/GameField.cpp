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



void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	NormalizedCellPadding = FMath::RoundToDouble(((TileSize + CellPadding) / TileSize) * 100) / 100;
}

void AGameField::BeginPlay()
{
	Super::BeginPlay();

	if (TileArray.Num() == 0)
	{
		GenerateField();
		// Chiama la generazione degli ostacoli, ad esempio con il 15% di ostacoli
		GenerateObstacles(0.15f);
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

	ATile* LocalSelectedTile = TileMap[Position];
	// Controlla se la tile è vuota
	if (!LocalSelectedTile || LocalSelectedTile->GetTileStatus() != ETileStatus::EMPTY)
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
		NewUnit->SetPlayerOwner(Player);
		// Aggiorna la posizione della griglia per la nuova unità:
		NewUnit->SetGridPosition(Position.X, Position.Y);
		UE_LOG(LogTemp, Warning, TEXT("Unità ID=%d posizionata in (%f, %f)"),
			NewUnit->GetGameUnitID(), NewUnit->GetGridPosition().X, NewUnit->GetGridPosition().Y);

		// Aggiorna lo stato della tile: ora è occupata dall'unità appena spawnata
		LocalSelectedTile->SetTileStatus(Player, ETileStatus::OCCUPIED, NewUnit);

		// Aggiungi l'unità nella mappa delle unità
		int32 NewUnitKey = GameUnitMap.Num();
		GameUnitMap.Add(NewUnitKey, NewUnit);

		UE_LOG(LogTemp, Log, TEXT("Unità di tipo %s spawnata in %s per il player %d"),
			*T::StaticClass()->GetName(), *Position.ToString(), Player);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawn dell'unità di tipo %s fallito in posizione %s"),
			*T::StaticClass()->GetName(), *Position.ToString());
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

void AGameField::GenerateObstacles(float ObstaclePercentage)
{
	//UE_LOG(LogTemp, Warning, TEXT("GenerateObstacles chiamata con perc=%f"), ObstaclePercentage);

	int32 TotalCells = Size * Size;
	int32 ObstaclesToPlace = FMath::RoundToInt(TotalCells * ObstaclePercentage);

	int32 Placed = 0;
	while (Placed < ObstaclesToPlace)
	{
		int32 RandIndex = FMath::RandRange(0, TileArray.Num() - 1);
		ATile* RandTile = TileArray[RandIndex];
		if (RandTile && RandTile->GetTileStatus() == ETileStatus::EMPTY)
		{
			RandTile->SetTileStatus(-1, ETileStatus::OBSTACLE, nullptr);
			RandTile->SetTileMaterial();
			//UE_LOG(LogTemp, Warning, TEXT("Piazzato ostacolo sulla tile %s"), *RandTile->GetGridPosition().ToString());
			Placed++;
		}
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




void AGameField::MoveUnit(AGameUnit* Unit, const FVector2D& NewPos, TFunction<void()> OnMovementFinished)
{
	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveUnit: Unità nulla!"));
		return;
	}

	// Calcola il percorso da seguire
	FVector2D StartPos = Unit->GetGridPosition();
	TArray<FVector2D> FoundPath = Unit->CalculatePath(NewPos);

	if (FoundPath.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveUnit: Nessun percorso trovato da %s a %s."), *StartPos.ToString(), *NewPos.ToString());
		return;
	}

	// Libera la tile di partenza
	if (TileMap.Contains(StartPos))
	{
		ATile* OldTile = TileMap[StartPos];
		OldTile->SetTileStatus(AGameField::NOT_ASSIGNED, ETileStatus::EMPTY, nullptr);
	}

	// Copia il percorso in uno smart pointer per mantenerlo valido nella lambda
	TSharedPtr<TArray<FVector2D>> PathPtr = MakeShared<TArray<FVector2D>>(FoundPath);

	// Usa dei TSharedPtr per l'indice corrente e il TimerHandle
	TSharedPtr<int32> CurrentStepPtr = MakeShared<int32>(0);
	TSharedPtr<FTimerHandle> TimerHandlePtr = MakeShared<FTimerHandle>();

	// Crea una lambda che gestisce il movimento step-by-step
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this, Unit, PathPtr, NewPos, CurrentStepPtr, TimerHandlePtr, OnMovementFinished]() mutable
		{
			// Se l'unità è stata distrutta, ferma il timer
			if (!Unit || !Unit->IsValidLowLevel())
			{
				GetWorld()->GetTimerManager().ClearTimer(*TimerHandlePtr);
				return;
			}

			// Se ci sono ancora passi da compiere nel percorso
			if (*CurrentStepPtr < PathPtr->Num())
			{
				// Ottieni la prossima cella del percorso
				FVector2D CellPos = (*PathPtr)[*CurrentStepPtr];
				FVector StepPos = GetRelativePositionByXYPosition(CellPos.X, CellPos.Y);

				// Aggiorna la posizione dell'unità (mantenendo l'altezza attuale)
				Unit->SetActorLocation(FVector(StepPos.X, StepPos.Y, Unit->GetActorLocation().Z));

				// Se siamo all'ultimo passo, aggiorna la tile di destinazione e la posizione in griglia
				if (*CurrentStepPtr == PathPtr->Num() - 1)
				{
					if (TileMap.Contains(NewPos))
					{
						ATile* DestTile = TileMap[NewPos];
						DestTile->SetTileStatus(Unit->GetPlayerOwner(), ETileStatus::OCCUPIED, Unit);
					}
					Unit->SetGridPosition(NewPos.X, NewPos.Y);

					// Movimento completato: chiama il callback se esiste
					if (OnMovementFinished)
					{
						OnMovementFinished();
					}
				}

				// Incrementa l'indice per il prossimo step
				(*CurrentStepPtr)++;
			}
			else
			{
				// Percorso completato: ferma il timer
				GetWorld()->GetTimerManager().ClearTimer(*TimerHandlePtr);
			}
		});

	// Avvia il timer: la lambda verrà eseguita ogni 0.1 secondi finché non viene fermata
	GetWorld()->GetTimerManager().SetTimer(*TimerHandlePtr, TimerDelegate, 0.1f, true);
}







void AGameField::AttackUnit(AGameUnit* Attacker, const FVector2D& TargetPos)
{
	if (!Attacker)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Attacker is null"));
		return;
	}

	if (!TileMap.Contains(TargetPos))
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackUnit: TargetPos %s is not valid"), *TargetPos.ToString());
		return;
	}

	ATile* TargetTile = TileMap[TargetPos];
	if (!TargetTile || TargetTile->GetTileStatus() != ETileStatus::OCCUPIED)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackUnit: No unit to attack at %s"), *TargetPos.ToString());
		return;
	}

	AGameUnit* Defender = TargetTile->GetGameUnit();
	if (!Defender)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Defender is null"));
		return;
	}

	// Verifica se TargetPos è effettivamente attaccabile dall'Attacker
	TArray<FVector2D> PossibleAttacks = Attacker->CalculateAttackMoves();
	if (!PossibleAttacks.Contains(TargetPos))
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Target cell %s is not in attack range"), *TargetPos.ToString());
		return;
	}

	// Calcola il danno casuale tra DamageMin e DamageMax dell'unità attaccante
	int32 Damage = FMath::RandRange(Attacker->GetDamageMin(), Attacker->GetDamageMax());
	UE_LOG(LogTemp, Log, TEXT("AttackUnit: Unit %d attacks unit %d for %d damage"),
		Attacker->GetGameUnitID(), Defender->GetGameUnitID(), Damage);

	// Sottrai i punti vita del difensore e aggiorna il widget (tramite TakeDamageUnit)
	Defender->TakeDamageUnit(Damage);

	// Se il difensore muore, rimuovilo dalla griglia e distruggilo
	if (Defender->IsDead())
	{
		UE_LOG(LogTemp, Log, TEXT("AttackUnit: Unit %d is destroyed"), Defender->GetGameUnitID());
		// Libera la tile
		TargetTile->SetTileStatus(AGameField::NOT_ASSIGNED, ETileStatus::EMPTY, nullptr);

		// Rimuovi il difensore dalla mappa delle unità
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

	// Se l’attaccante è uno Sniper, gestisci il contrattacco (se le condizioni sono verificate)
	if (Attacker->GetGameUnitType() == EGameUnitType::SNIPER)
	{
		ASniper* Sniper = Cast<ASniper>(Attacker);
		if (Sniper)
		{
			Sniper->HandleCounterAttack(Defender);
		}
	}
}


void AGameField::ShowLegalMovesForUnit(AGameUnit* Unit)
{
	UE_LOG(LogTemp, Warning, TEXT("Chiamato ShowLegalMovesForUnit"));
	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowLegalMovesForUnit: Unità nulla."));
		return;
	}

	AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GField)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowLegalMovesForUnit: GameMode o GameField non trovato."));
		return;
	}

	// Resetta lo stato di evidenziazione di tutte le tile
	ResetGameStatusField();

	// Se l'unità non ha ancora mosso, evidenzia le celle di movimento
	if (!Unit->bHasMoved)
	{
		TArray<FVector2D> LegalMoves = Unit->CalculateLegalMoves();
		TArray<FVector2D> ValidMoves;
		for (const FVector2D& MovePos : LegalMoves)
		{
			if (IsValidPosition(MovePos))
			{
				ATile* Tile = TileMap[MovePos];
				if (Tile && Tile->GetTileStatus() == ETileStatus::EMPTY)
				{
					ValidMoves.Add(MovePos);
				}
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Numero di mosse legali trovate: %d"), ValidMoves.Num());
		for (const FVector2D& MovePos : ValidMoves)
		{
			if (IsValidPosition(MovePos))
			{
				ATile* Tile = TileMap[MovePos];
				if (Tile)
				{
					Tile->SetTileGameStatus(ETileGameStatus::LEGAL_MOVE);
				}
			}
		}
	}

}

void AGameField::ShowLegalAttackOptionsForUnit(AGameUnit* Unit)
{
	// Non è necessario resettare qui se ShowLegalMovesForUnit l'ha già fatto
	TArray<FVector2D> AttackMoves = Unit->CalculateAttackMoves();
	TArray<FVector2D> ValidAttacks;
	for (const FVector2D& AttackPos : AttackMoves)
	{
		if (IsValidPosition(AttackPos))
		{
			ATile* Tile = TileMap[AttackPos];
			// Considera valida la mossa di attacco se la tile è occupata da un nemico
			if (Tile && Tile->GetTileStatus() == ETileStatus::OCCUPIED)
			{
				AGameUnit* Target = Tile->GetGameUnit();
				if (Target && Target->GetPlayerOwner() != Unit->GetPlayerOwner())
				{
					ValidAttacks.Add(AttackPos);
				}
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Numero di attacchi validi trovati: %d"), ValidAttacks.Num());
	for (const FVector2D& AttackPos : ValidAttacks)
	{
		if (IsValidPosition(AttackPos))
		{
			ATile* Tile = TileMap[AttackPos];
			if (Tile)
			{
				Tile->SetTileGameStatus(ETileGameStatus::CAN_ATTACK);
			}
		}
	}
}










