#include "GameField.h"
#include "Kismet/GameplayStatics.h"
#include "GameUnit.h"
#include "Sniper.h"
#include "Brawler.h"
#include "MovesPanel.h"
#include "MoveHistoryWidget.h"
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
	FVector RelativeLocation = Location - GetActorLocation();

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
	if (TileMap.Contains(FVector2D(X, Y)))
	{
		ATile* TheTile = TileMap[FVector2D(X, Y)];
		if (TheTile)
		{
			char LetterChar = 'A' + X;
			FString LetterString = FString::Printf(TEXT("%c"), LetterChar);

			int32 NumberValue = Y + 1;
			FString NumberString = FString::FromInt(NumberValue);

			if (TheTile->TileTextLetter)
			{
				TheTile->TileTextLetter->SetText(FText::FromString(LetterString));
			}

			if (TheTile->TileTextNumber)
			{
				TheTile->TileTextNumber->SetText(FText::FromString(NumberString));
			}

			//UE_LOG(LogTemp, Log, TEXT("Tile (%d, %d): Letter = %s, Number = %s"),X, Y, *LetterString, *NumberString);
		}
	}
}

template<typename T>
void AGameField::GenerateGameUnit(FVector2D Position, int32 Player)
{
	if (!TileMap.Contains(Position))
	{
		//UE_LOG(LogTemp, Warning, TEXT("No tile found at position %s!"), *Position.ToString());
		return;
	}

	ATile* LocalSelectedTile = TileMap[Position];
	if (!LocalSelectedTile || LocalSelectedTile->GetTileStatus() != ETileStatus::EMPTY)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Tile at position %s is not empty!"), *Position.ToString());
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		//UE_LOG(LogTemp, Error, TEXT("World not found!"));
		return;
	}

	FVector SpawnLocation = GetRelativePositionByXYPosition(Position.X, Position.Y);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	T* NewUnit = World->SpawnActor<T>(T::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (NewUnit)
	{
		NewUnit->SetPlayerOwner(Player);
		NewUnit->SetGridPosition(Position.X, Position.Y);
		//UE_LOG(LogTemp, Warning, TEXT("Unit ID=%d spawned at (%f, %f)"),NewUnit->GetGameUnitID(), NewUnit->GetGridPosition().X, NewUnit->GetGridPosition().Y);

		LocalSelectedTile->SetTileStatus(Player, ETileStatus::OCCUPIED, NewUnit);

		int32 NewUnitKey = GameUnitMap.Num();
		GameUnitMap.Add(NewUnitKey, NewUnit);

		//UE_LOG(LogTemp, Log, TEXT("Unit of type %s spawned at %s for player %d"),*T::StaticClass()->GetName(), *Position.ToString(), Player);
	}

}


// Explicit template instantiations
template void AGameField::GenerateGameUnit<ASniper>(FVector2D Position, int32 Player);
template void AGameField::GenerateGameUnit<ABrawler>(FVector2D Position, int32 Player);


inline bool AGameField::IsValidPosition(const FVector2D Position) const
{
	return 0 <= Position.X && Position.X < Size && 0 <= Position.Y && Position.Y < Size;
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

	if (TileMap.Contains(Position))
	{
		ATile* Tile = TileMap[Position];
		if (Tile && Tile->GetGameUnit())
		{
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

bool AGameField::IsGridConnected() const
{
	FVector2D StartPos(-1, -1);
	for (ATile* Tile : TileArray)
	{
		if (Tile && Tile->GetTileStatus() != ETileStatus::OBSTACLE)
		{
			StartPos = Tile->GetGridPosition();
			break;
		}
	}
	if (StartPos.X < 0)
	{
		return true;
	}

	// BFS
	TSet<FVector2D> Visited;
	TQueue<FVector2D> Queue;
	Queue.Enqueue(StartPos);
	Visited.Add(StartPos);

	TArray<FVector2D> Directions = { FVector2D(1, 0), FVector2D(-1, 0), FVector2D(0, 1), FVector2D(0, -1) };

	while (!Queue.IsEmpty())
	{
		FVector2D Current;
		Queue.Dequeue(Current);
		for (const FVector2D& Dir : Directions)
		{
			FVector2D Neighbor = Current + Dir;
			if (IsValidPosition(Neighbor) && !Visited.Contains(Neighbor))
			{
				ATile* NeighborTile = nullptr;
				if (TileMap.Contains(Neighbor))
				{
					NeighborTile = TileMap[Neighbor];
				}
				if (NeighborTile && NeighborTile->GetTileStatus() != ETileStatus::OBSTACLE)
				{
					Visited.Add(Neighbor);
					Queue.Enqueue(Neighbor);
				}
			}
		}
	}
	int32 TotalFreeCells = 0;
	for (ATile* Tile : TileArray)
	{
		if (Tile && Tile->GetTileStatus() != ETileStatus::OBSTACLE)
		{
			TotalFreeCells++;
		}
	}

	return Visited.Num() == TotalFreeCells;
}

void AGameField::GenerateObstacles(float ObstaclePercentage)
{
	int32 TotalCells = Size * Size;
	int32 ObstaclesToPlace = FMath::RoundToInt(TotalCells * ObstaclePercentage);
	int32 Placed = 0;
	int32 Attempts = 0;
	int32 MaxAttempts = ObstaclesToPlace * 10;

	while (Placed < ObstaclesToPlace && Attempts < MaxAttempts)
	{
		Attempts++;
		int32 RandIndex = FMath::RandRange(0, TileArray.Num() - 1);
		ATile* RandTile = TileArray[RandIndex];
		if (RandTile && RandTile->GetTileStatus() == ETileStatus::EMPTY)
		{
			RandTile->SetTileStatus(-1, ETileStatus::OBSTACLE, nullptr);
			RandTile->SetTileMaterial();

			if (!IsGridConnected())
			{
				RandTile->SetTileStatus(AGameField::NOT_ASSIGNED, ETileStatus::EMPTY, nullptr);
				RandTile->SetTileMaterial();
			}
			else
			{
				Placed++;
			}
		}
	}

	if (Attempts >= MaxAttempts)
	{
		//UE_LOG(LogTemp, Warning, TEXT("GenerateObstacles: maximum attempts reached, obstacles placed: %d"), Placed);
	}
}



void AGameField::ResetField()
{
		SelectedTile = FVector2D::ZeroVector;
		LegalMovesArray.Empty();

		for (ATile* Tile : TileArray)
		{
			if (Tile)
			{
				Tile->SetTileGameStatus(ETileGameStatus::FREE);
				Tile->SetTileStatus(NOT_ASSIGNED, ETileStatus::EMPTY, nullptr);

				Tile->SetTileMaterial();
			}
		}

		for (auto& Pair : GameUnitMap)
		{
			if (Pair.Value)
			{
				Pair.Value->Destroy();
			}
		}
		GameUnitMap.Empty();

		UE_LOG(LogTemp, Log, TEXT("GameField reset: all tiles reset and units eliminated."));

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
		//UE_LOG(LogTemp, Warning, TEXT("MoveUnit: Unit is null!"));
		return;
	}

	FVector2D StartPos = Unit->GetGridPosition();
	TArray<FVector2D> FoundPath = Unit->CalculatePath(NewPos);

	if (FoundPath.Num() == 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MoveUnit: No path found from %s to %s."), *StartPos.ToString(), *NewPos.ToString());
		return;
	}

	if (TileMap.Contains(StartPos))
	{
		ATile* OldTile = TileMap[StartPos];
		OldTile->SetTileStatus(AGameField::NOT_ASSIGNED, ETileStatus::EMPTY, nullptr);
	}

	TSharedPtr<TArray<FVector2D>> PathPtr = MakeShared<TArray<FVector2D>>(FoundPath);

	TSharedPtr<int32> CurrentStepPtr = MakeShared<int32>(0);
	TSharedPtr<FTimerHandle> TimerHandlePtr = MakeShared<FTimerHandle>();
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this, Unit, PathPtr, StartPos, NewPos, CurrentStepPtr, TimerHandlePtr, OnMovementFinished]() mutable
		{
			if (!Unit || !Unit->IsValidLowLevel())
			{
				GetWorld()->GetTimerManager().ClearTimer(*TimerHandlePtr);
				return;
			}

			if (*CurrentStepPtr < PathPtr->Num())
			{
				FVector2D CellPos = (*PathPtr)[*CurrentStepPtr];
				FVector StepPos = GetRelativePositionByXYPosition(CellPos.X, CellPos.Y);

				Unit->SetActorLocation(FVector(StepPos.X, StepPos.Y, Unit->GetActorLocation().Z));

				if (*CurrentStepPtr == PathPtr->Num() - 1)
				{
					if (TileMap.Contains(NewPos))
					{
						ATile* DestTile = TileMap[NewPos];
						DestTile->SetTileStatus(Unit->GetPlayerOwner(), ETileStatus::OCCUPIED, Unit);
					}
					Unit->SetGridPosition(NewPos.X, NewPos.Y);
					FGameMove NewMove;
					NewMove.PlayerID = (Unit->GetPlayerOwner() == 0) ? TEXT("HP") : TEXT("AI");
					NewMove.UnitType = (Unit->GetGameUnitType() == EGameUnitType::SNIPER) ? TEXT("S") : TEXT("B");
					NewMove.FromCell = ConvertGridPosToCellString(StartPos);
					NewMove.ToCell = ConvertGridPosToCellString(NewPos);
					NewMove.bIsAttack = false;
					
					AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
					if (GM && GM->MovesPanel)
					{
						GM->MovesPanel->AddMoveToPanel(NewMove);
					}
					if (OnMovementFinished)
					{
						OnMovementFinished();
					}
					
				}
				(*CurrentStepPtr)++;
			}
			else
			{
				GetWorld()->GetTimerManager().ClearTimer(*TimerHandlePtr);
			}
		});

	GetWorld()->GetTimerManager().SetTimer(*TimerHandlePtr, TimerDelegate, 0.1f, true);
}

void AGameField::AttackUnit(AGameUnit* Attacker, const FVector2D& TargetPos)
{
	if (!Attacker)
	{
		//UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Attacker is null"));
		return;
	}

	if (!TileMap.Contains(TargetPos))
	{
		//UE_LOG(LogTemp, Warning, TEXT("AttackUnit: TargetPos %s is not valid"), *TargetPos.ToString());
		return;
	}

	ATile* TargetTile = TileMap[TargetPos];
	if (!TargetTile || TargetTile->GetTileStatus() != ETileStatus::OCCUPIED)
	{
		//UE_LOG(LogTemp, Warning, TEXT("AttackUnit: No unit to attack at %s"), *TargetPos.ToString());
		return;
	}

	AGameUnit* Defender = TargetTile->GetGameUnit();
	if (!Defender)
	{
		//UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Defender is null"));
		return;
	}

	TArray<FVector2D> PossibleAttacks = Attacker->CalculateAttackMoves();
	if (!PossibleAttacks.Contains(TargetPos))
	{
		//UE_LOG(LogTemp, Warning, TEXT("AttackUnit: Target cell %s is not in attack range"), *TargetPos.ToString());
		return;
	}


	int32 Damage = FMath::RandRange(Attacker->GetDamageMin(), Attacker->GetDamageMax());
	//UE_LOG(LogTemp, Log, TEXT("AttackUnit: Unit %d attacks unit %d for %d damage"), Attacker->GetGameUnitID(), Defender->GetGameUnitID(), Damage);

	FGameMove AttackMove;
	AttackMove.PlayerID = (Attacker->GetPlayerOwner() == 0) ? TEXT("HP") : TEXT("AI");
	AttackMove.UnitType = (Attacker->GetGameUnitType() == EGameUnitType::SNIPER) ? TEXT("S") : TEXT("B");
	AttackMove.TargetCell = ConvertGridPosToCellString(TargetPos);
	AttackMove.Damage = Damage;
	AttackMove.bIsAttack = true;
	
	AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
	if (GM && GM->MovesPanel)
	{
		GM->MovesPanel->AddMoveToPanel(AttackMove);
	}

	Defender->TakeDamageUnit(Damage);

	if (Defender->IsDead())
	{
		//UE_LOG(LogTemp, Log, TEXT("AttackUnit: Unit %d is destroyed"), Defender->GetGameUnitID());
		TargetTile->SetTileStatus(AGameField::NOT_ASSIGNED, ETileStatus::EMPTY, nullptr);

		for (auto It = GameUnitMap.CreateIterator(); It; ++It)
		{
			if (It.Value() == Defender)
			{
				It.RemoveCurrent();
				break;
			}
		}
		Defender->Destroy();

		bool bUnitsLeft = false;
		for (auto& UnitPair : GameUnitMap)
		{
			if (UnitPair.Value && UnitPair.Value->GetPlayerOwner() == Defender->GetPlayerOwner())
			{
				bUnitsLeft = true;
				break;
			}
		}

		if (!bUnitsLeft)
		{
			
			if (GM)
			{
				GM->EndGame();
			}
		}
	}

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
	if (Unit->IsDead())
	{
		//UE_LOG(LogTemp, Warning, TEXT("ShowLegalMovesForUnit: Unit is dead!"));
		return;
	}
	
	AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GField)
	{
		//UE_LOG(LogTemp, Warning, TEXT("ShowLegalMovesForUnit: GameMode or GameField not found."));
		return;
	}

	ResetGameStatusField();

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
		//UE_LOG(LogTemp, Warning, TEXT("Number of legal moves found: %d"), ValidMoves.Num());
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
	TArray<FVector2D> AttackMoves = Unit->CalculateAttackMoves();
	TArray<FVector2D> ValidAttacks;
	for (const FVector2D& AttackPos : AttackMoves)
	{
		if (IsValidPosition(AttackPos))
		{
			ATile* Tile = TileMap[AttackPos];
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
	//UE_LOG(LogTemp, Warning, TEXT("Number of valid attacks found: %d"), ValidAttacks.Num());
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










