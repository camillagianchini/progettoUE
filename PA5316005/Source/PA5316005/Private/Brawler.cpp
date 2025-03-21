#include "Brawler.h"
#include "AWGameMode.h"
#include "Tile.h"
#include "GameField.h"


ABrawler::ABrawler()
{
	// Imposta i parametri predefiniti per il Brawler secondo le specifiche:
	// Movimento: max 6 celle, Range attacco: 1, Danno: 1–6, Vita: 40
	SetGameUnitType(EGameUnitType::BRAWLER);
	SetMovementRange(6);
	SetAttackRange(1);
	SetDamage(1, 6);
	SetHitPoints(40);
}

void ABrawler::BeginPlay()
{
	Super::BeginPlay();


	SetGameUnitID();
	UE_LOG(LogTemp, Warning, TEXT("ABrawler::BeginPlay - Type=%d, ID=%d"),
		(int32)GetGameUnitType(), GetGameUnitID());
}

// In ABrawler.cpp
TArray<FVector2D> ABrawler::CalculateAttackMoves() const
{
    TArray<FVector2D> AttackableCells;
    AGameField* GF = GameMode->GField;
    FVector2D MyPos = GetGridPosition();

    // Solo 4 direzioni ortogonali (su, giù, sinistra, destra)
    const TArray<FVector2D> Directions = {
        FVector2D(1, 0),
        FVector2D(-1, 0),
        FVector2D(0, 1),
        FVector2D(0, -1)
    };

    for (const FVector2D& Dir : Directions)
    {
        FVector2D Candidate = MyPos + Dir;
        if (GF->IsValidPosition(Candidate) && GF->TileMap.Contains(Candidate))
        {
            ATile* Tile = GF->TileMap[Candidate];
            if (Tile)
            {
                AGameUnit* OtherUnit = Tile->GetGameUnit();
                if (OtherUnit && OtherUnit->GetPlayerOwner() != GetPlayerOwner())
                {
                    AttackableCells.Add(Candidate);
                }
            }
        }
    }

    return AttackableCells;
}


