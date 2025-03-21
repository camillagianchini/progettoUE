#include "Sniper.h"
#include "AWGameMode.h"
#include "Tile.h"
#include "GameField.h"

ASniper::ASniper()
{
	// Imposta i parametri predefiniti per lo Sniper secondo le specifiche:
	// Movimento: max 3 celle, Range attacco: max 10, Danno: 4–8, Vita: 20
	SetGameUnitType(EGameUnitType::SNIPER);
	SetMovementRange(3);
	SetAttackRange(10);
	SetDamage(4, 8);
	SetHitPoints(20);
}

void ASniper::BeginPlay()
{
	Super::BeginPlay();
	MovementRange = 3;

	// Assegna un ID univoco all'unità
	SetGameUnitID();
}

void ASniper::HandleCounterAttack(AGameUnit* AttackedUnit)
{
	if (!AttackedUnit)
	{
		return;
	}

	bool bShouldCounterAttack = false;

	// Se l'unità attaccata è uno sniper, attiva il contrattacco.
	if (AttackedUnit->GetGameUnitType() == EGameUnitType::SNIPER)
	{
		bShouldCounterAttack = true;
	}
	// Se l'unità attaccata è un brawler e si trova adiacente (distanza di Manhattan pari a 1)
	else if (AttackedUnit->GetGameUnitType() == EGameUnitType::BRAWLER)
	{
		FVector2D AttackerPos = GetGridPosition();
		FVector2D TargetPos = AttackedUnit->GetGridPosition();
		int32 dx = FMath::Abs(AttackerPos.X - TargetPos.X);
		int32 dy = FMath::Abs(AttackerPos.Y - TargetPos.Y);
		if ((dx + dy) == 1)
		{
			bShouldCounterAttack = true;
		}
	}

	if (bShouldCounterAttack)
	{
		// Calcola un danno da contrattacco random tra 1 e 3
		int32 CounterDamage = FMath::RandRange(1, 3);
		TakeDamageUnit(CounterDamage);
		UE_LOG(LogTemp, Log, TEXT("Sniper (ID: %d) riceve danno da contrattacco: %d"), GetGameUnitID(), CounterDamage);
	}
}

// In ASniper.cpp
TArray<FVector2D> ASniper::CalculateAttackMoves() const
{
	TArray<FVector2D> AttackableCells;
	AGameField* GF = GameMode->GField;
	FVector2D MyPos = GetGridPosition();

	for (int32 x = 0; x < GameMode->FieldSize; x++)
	{
		for (int32 y = 0; y < GameMode->FieldSize; y++)
		{
			FVector2D Candidate(x, y);
			int32 ManhattanDist = FMath::Abs(Candidate.X - MyPos.X) + FMath::Abs(Candidate.Y - MyPos.Y);
			if (ManhattanDist > 0 && ManhattanDist <= AttackRange)
			{
				if (GF->TileMap.Contains(Candidate))
				{
					ATile* Tile = GF->TileMap[Candidate];
					if (Tile && Tile->GetTileStatus() == ETileStatus::OCCUPIED)
					{
						AGameUnit* OtherUnit = Tile->GetGameUnit();
						if (OtherUnit && OtherUnit->GetPlayerOwner() != GetPlayerOwner())
						{
							AttackableCells.Add(Candidate);
						}
					}
				}
			}
		}
	}

	return AttackableCells;
}




