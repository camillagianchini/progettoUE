#include "GameUnit.h"
#include "GameField.h"
#include "AWGameMode.h"
#include "GameField.h"
#include "UObject/ConstructorHelpers.h"

int32 AGameUnit::NewGameUnitID = 0;

// Sets default values
AGameUnit::AGameUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	// Creazione dei componenti base
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	// Imposta il componente di root e attacca il mesh
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	// Inizializza le proprietà della GameUnit
	GameUnitGridPosition = FVector2D(-1, -1);
	PlayerOwner = -1;
	GameUnitID = -100;
	HitPoints = 0;
	MovementRange = 0;
	AttackRange = 0;
	DamageMin = 0;
	DamageMax = 0;
	GameUnitType = EGameUnitType::SNIPER; // Valore di default; può essere modificato tramite setter
	GameMode = nullptr;
}

// Called every frame
void AGameUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the game starts or when spawned
void AGameUnit::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());

}

void AGameUnit::SetGameUnitID()
{
	// Incrementa l'ID e lo assegna all'unità
	GameUnitID = NewGameUnitID++;
}

void AGameUnit::SetPlayerOwner(int32 NewPlayerOwner)
{
	PlayerOwner = NewPlayerOwner;
}

void AGameUnit::SetGridPosition(const double InX, const double InY)
{
	GameUnitGridPosition.Set(InX, InY);
}

void AGameUnit::SetGameUnitType(EGameUnitType NewType)
{
	GameUnitType = NewType;
}

void AGameUnit::SetHitPoints(int32 NewHitPoints)
{
	HitPoints = NewHitPoints;
}

void AGameUnit::SetMovementRange(int32 NewRange)
{
	MovementRange = NewRange;
}

void AGameUnit::SetAttackRange(int32 NewAttackRange)
{
	AttackRange = NewAttackRange;
}

void AGameUnit::SetDamage(int32 NewDamageMin, int32 NewDamageMax)
{
	DamageMin = NewDamageMin;
	DamageMax = NewDamageMax;
}

int32 AGameUnit::GetGameUnitID() const
{
	return GameUnitID;
}

int32 AGameUnit::GetPlayerOwner() const
{
	return PlayerOwner;
}

FVector2D AGameUnit::GetGridPosition() const
{
	return GameUnitGridPosition;
}

EGameUnitType AGameUnit::GetGameUnitType() const
{
	return GameUnitType;
}

int32 AGameUnit::GetHitPoints() const
{
	return HitPoints;
}

int32 AGameUnit::GetMovementRange() const
{
	return MovementRange;
}

int32 AGameUnit::GetAttackRange() const
{
	return AttackRange;
}

int32 AGameUnit::GetDamageMin() const
{
	return DamageMin;
}

int32 AGameUnit::GetDamageMax() const
{
	return DamageMax;
}



TArray<FVector2D> AGameUnit::CalculateLegalMoves()
{
	TArray<FVector2D> Result;
	if (!GameMode || !GameMode->GField) return Result;

	// BFS structures
	TQueue<FVector2D> Frontier;
	TSet<FVector2D> Visited;
	TMap<FVector2D, int32> DistMap;

	FVector2D StartPos = GetGridPosition();
	Frontier.Enqueue(StartPos);
	Visited.Add(StartPos);
	DistMap.Add(StartPos, 0);

	while (!Frontier.IsEmpty())
	{
		FVector2D Current;
		Frontier.Dequeue(Current);

		int32 CurrentDist = DistMap[Current];

		// 4 direzioni
		static TArray<FVector2D> Dirs = {
			FVector2D(1,0), FVector2D(-1,0),
			FVector2D(0,1), FVector2D(0,-1)
		};

		for (auto& Dir : Dirs)
		{
			FVector2D Next = Current + Dir;
			int32 NextDist = CurrentDist + 1;
			if (NextDist <= MovementRange)
			{
				// Se stiamo controllando la cella di partenza o no
				bool bIsStart = (Next == StartPos);
				// Controlla se la cella Next è valida
				if (IsValidGridCell(Next, bIsStart) && !Visited.Contains(Next))
				{
					// Se non è la cella di partenza, aggiungila alle mosse
					if (!bIsStart)
					{
						Result.Add(Next);
					}
					Frontier.Enqueue(Next);
					Visited.Add(Next);
					DistMap.Add(Next, NextDist);
				}
			}
		}
	}
	return Result;
}


bool AGameUnit::IsValidGridCell(const FVector2D& CellPos, bool bIsStart) const
{
	if (!GameMode || !GameMode->GField)
		return false;

	// Controllo limiti della griglia
	AGameField* GF = GameMode->GField;
	if (CellPos.X < 0 || CellPos.X >= GF->Size ||
		CellPos.Y < 0 || CellPos.Y >= GF->Size)
	{
		return false;
	}

	// Ottieni la tile
	ATile* Tile = GF->GetTileMap()[CellPos];
	if (!Tile)
		return false;

	// Se è la cella di partenza, consentila anche se "OCCUPIED" dalla stessa unità
	if (bIsStart)
	{
		// Se c'è un'unità su questa tile...
		if (Tile->GetTileStatus() == ETileStatus::OCCUPIED)
		{
			AGameUnit* Occupant = Tile->GetGameUnit();
			// ... e NON è la stessa unità, blocca
			if (Occupant != this)
				return false;
		}
		return true; // Ok, è la cella di partenza occupata da me stesso
	}
	else
	{
		// Per le celle successive, devono essere strictly EMPTY
		return (Tile->GetTileStatus() == ETileStatus::EMPTY);
	}
}




TArray<FVector2D> AGameUnit::CalculateAttackMoves() const
{
	TArray<FVector2D> AttackMoves;
	// Utilizza AttackRange per calcolare le posizioni di attacco
	for (int32 Offset = 1; Offset <= AttackRange; Offset++)
	{
		AttackMoves.Add(FVector2D(GameUnitGridPosition.X + Offset, GameUnitGridPosition.Y));
		AttackMoves.Add(FVector2D(GameUnitGridPosition.X - Offset, GameUnitGridPosition.Y));
		AttackMoves.Add(FVector2D(GameUnitGridPosition.X, GameUnitGridPosition.Y + Offset));
		AttackMoves.Add(FVector2D(GameUnitGridPosition.X, GameUnitGridPosition.Y - Offset));
	}
	return AttackMoves;
}


void AGameUnit::TakeDamageUnit(int32 DamageAmount)
{
	HitPoints -= DamageAmount;
	if (HitPoints < 0)
	{
		HitPoints = 0;
	}
}

bool AGameUnit::IsDead() const
{
	return HitPoints <= 0;
}




