#include "GameUnit.h"
#include "GameField.h"
#include "Tile.h"
#include "Math/UnrealMathUtility.h"

AGameUnit::AGameUnit()
{
	PrimaryActorTick.bCanEverTick = false;

	// Valori di default (potranno essere sovrascritti dalle classi figlie)
	MaxMovement = 3;
	AttackType = EAttackType::Melee;
	AttackRange = 1;
	DamageMin = 1;
	DamageMax = 6;
	HitPoints = 20;

	GridPosition = FVector2D(-1, -1);
	GameFieldRef = nullptr;
}

bool AGameUnit::MoveUnit(const FVector2D& Destination)
{
	if (!GameFieldRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("AGameUnit::MoveUnit: GameFieldRef is null!"));
		return false;
	}

	// 1) Calcola distanza Manhattan fra la posizione attuale e la destinazione
	int32 Dist = CalculateDistance(GridPosition, Destination);
	if (Dist > MaxMovement)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot move that far. Dist=%d, MaxMovement=%d"),
			*GetName(), Dist, MaxMovement);
		return false;
	}

	// 2) Verifica che tutte le celle intermedie siano percorribili (no ostacoli, no unità).
	//    In un progetto più avanzato useresti un BFS/A* per il percorso, 
	//    qui facciamo un check semplificato "lineare" solo se dist <= MaxMovement.
	//    (Se vuoi un pathfinding vero, implementalo qui o in un'altra funzione.)

	// Movimento solo ortogonale, no diagonali:
	int32 StepX = (Destination.X > GridPosition.X) ? 1 : -1;
	int32 StepY = (Destination.Y > GridPosition.Y) ? 1 : -1;

	// Ci muoviamo in X finché non ci allineiamo, poi in Y (o viceversa).
	// Per semplicità, muoviamo X e Y separatamente.
	// (Attenzione: se Dist > 1 e c'è un ostacolo in mezzo, qui non lo gestiamo in modo "pathfinding".)

	FVector2D Current = GridPosition;

	// Prima muoviamo sull'asse X
	while (FMath::Abs(Current.X - Destination.X) > 0)
	{
		Current.X += StepX;
		if (!GameFieldRef->IsValidCoordinate(Current))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid coordinate during movement."));
			return false;
		}
		ATile* NextTile = GameFieldRef->TileMap[Current];
		if (!NextTile->IsWalkable())
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot move: found an obstacle or occupied tile at X movement"));
			return false;
		}
	}

	// Poi muoviamo sull'asse Y
	while (FMath::Abs(Current.Y - Destination.Y) > 0)
	{
		Current.Y += StepY;
		if (!GameFieldRef->IsValidCoordinate(Current))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid coordinate during movement."));
			return false;
		}
		ATile* NextTile = GameFieldRef->TileMap[Current];
		if (!NextTile->IsWalkable())
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot move: found an obstacle or occupied tile at Y movement"));
			return false;
		}
	}

	// Se arriviamo qui, significa che il percorso è libero.
	// Dobbiamo aggiornare la tile di partenza a EMPTY e la tile di arrivo a OCCUPIED,
	// e aggiornare la GridPosition dell'unità.
	// NB: in un progetto più completo, memorizzeresti la tile corrente e la svuoteresti.

	// Esempio:
	// Svuota la tile di partenza
	if (GameFieldRef->IsValidCoordinate(GridPosition))
	{
		ATile* OldTile = GameFieldRef->TileMap[GridPosition];
		if (OldTile && OldTile->GetTileStatus() == ETileStatus::OCCUPIED)
		{
			OldTile->SetTileStatus(-1, ETileStatus::EMPTY);
		}
	}

	// Occupiamo la tile di destinazione
	ATile* DestTile = GameFieldRef->TileMap[Destination];
	DestTile->SetTileStatus(-1, ETileStatus::OCCUPIED);

	// Aggiorniamo la posizione dell'unità
	GridPosition = Destination;

	// Aggiorniamo la posizione fisica dell'Actor
	FVector NewLocation = GameFieldRef->GetRelativeLocationByXYPosition(GridPosition.X, GridPosition.Y);
	SetActorLocation(NewLocation);

	UE_LOG(LogTemp, Log, TEXT("%s moved to [%f, %f] successfully."), *GetName(), GridPosition.X, GridPosition.Y);
	return true;
}

void AGameUnit::AttackUnit(AGameUnit* TargetUnit)
{
	if (!TargetUnit)
	{
		return;
	}

	// Controllo della distanza
	int32 Dist = CalculateDistance(GridPosition, TargetUnit->GridPosition);
	if (Dist > AttackRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot attack: target is out of range (Dist=%d, Range=%d)."),
			*GetName(), Dist, AttackRange);
		return;
	}

	// Calcolo danno
	int32 Damage = FMath::RandRange(DamageMin, DamageMax);
	UE_LOG(LogTemp, Log, TEXT("%s attacks %s for %d damage"), *GetName(), *TargetUnit->GetName(), Damage);

	TargetUnit->ReceiveDamage(Damage);

	// Nella classe base non gestiamo contrattacchi speciali.
	// (Lo Sniper lo farà in override.)
}

void AGameUnit::ReceiveDamage(int32 Damage)
{
	HitPoints -= Damage;
	UE_LOG(LogTemp, Log, TEXT("%s received %d damage (HP now %d)"), *GetName(), Damage, HitPoints);

	if (HitPoints <= 0)
	{
		Die();
	}
}

void AGameUnit::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("%s has died!"), *GetName());

	// Svuota la tile su cui era l'unità
	if (GameFieldRef && GameFieldRef->IsValidCoordinate(GridPosition))
	{
		ATile* Tile = GameFieldRef->TileMap[GridPosition];
		if (Tile && Tile->GetTileStatus() == ETileStatus::OCCUPIED)
		{
			Tile->SetTileStatus(-1, ETileStatus::EMPTY);
		}
	}

	Destroy();
}

int32 AGameUnit::CalculateDistance(const FVector2D& From, const FVector2D& To) const
{
	// Distanza Manhattan
	int32 DeltaX = FMath::Abs((int32)From.X - (int32)To.X);
	int32 DeltaY = FMath::Abs((int32)From.Y - (int32)To.Y);
	return DeltaX + DeltaY;
}




