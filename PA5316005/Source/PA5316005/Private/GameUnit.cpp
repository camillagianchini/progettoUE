#include "GameUnit.h"
#include "GameField.h"
#include "AWGameMode.h"
#include "GameField.h"
#include "UnitListWidget.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"

int32 AGameUnit::NewGameUnitID = 0;


AGameUnit::AGameUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	bHasMoved = false;
	bHasAttacked = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));


	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	GameUnitGridPosition = FVector2D(-1, -1);
	PlayerOwner = -1;
	GameUnitID = -100;
	HitPoints = 0;
	MovementRange = 0;
	AttackRange = 0;
	DamageMin = 0;
	DamageMax = 0;
	GameUnitType = EGameUnitType::SNIPER;
	GameMode = nullptr;
}

void AGameUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGameUnit::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());

}

TArray<FVector2D> AGameUnit::CalculateAttackMoves() const
{
	return TArray<FVector2D>();
}

void AGameUnit::SetGameUnitID()
{

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

// BFS
TArray<FVector2D> AGameUnit::CalculateLegalMoves()
{
	TArray<FVector2D> Result;

	AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GField)
	{
		return Result;
	}

	int32 FS = GM->FieldSize;

	TQueue<FVector2D> Frontier;
	TSet<FVector2D> Visited;
	TMap<FVector2D, int32> DistMap;

	Frontier.Enqueue(GetGridPosition());
	Visited.Add(GetGridPosition());
	DistMap.Add(GetGridPosition(), 0);

	static TArray<FVector2D> Dirs = {
		FVector2D(1, 0), FVector2D(-1, 0),
		FVector2D(0, 1), FVector2D(0, -1)
	};

	while (!Frontier.IsEmpty())
	{
		FVector2D Current;
		Frontier.Dequeue(Current);
		int32 CurrentDist = DistMap[Current];

		for (auto& Dir : Dirs)
		{
			FVector2D Next = Current + Dir;

			if (Next.X < 0 || Next.X >= FS || Next.Y < 0 || Next.Y >= FS)
			{
				continue;
			}

			int32 NextDist = CurrentDist + 1;
			if (NextDist <= MovementRange && !Visited.Contains(Next))
			{
				if (IsValidGridCell(Next, false))
				{
					Result.Add(Next);
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
	{
		//UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell: GameMode or GField is null for cell %s"), *CellPos.ToString());
		return false;
	}

	if (!GameMode->GField->IsValidPosition(CellPos))
	{
		//UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell: Cell %s is out of grid bounds"), *CellPos.ToString());
		return false;
	}

	AGameField* GF = GameMode->GField;
	if (!GF->TileMap.Contains(CellPos))
	{
		//UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell: Cell %s not found in TileMap"), *CellPos.ToString());
		return false;
	}

	ATile* Tile = GF->TileMap[CellPos];
	if (!Tile)
	{
		return false;
	}

	if (bIsStart)
	{
		if (Tile->GetGameUnit() == this)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Starting cell %s is valid: occupied by this unit (ID: %d)"),*CellPos.ToString(), this->GetGameUnitID());
			return true;
		}
	}

	if (Tile->GetTileStatus() == ETileStatus::OBSTACLE)
	{
		return false;
	}

	if (Tile->GetTileStatus() == ETileStatus::OCCUPIED)
	{
		if (bIsStart)
		{
			if (Tile->GetGameUnit() == this)
			{
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

// BFS
TArray<FVector2D> AGameUnit::CalculatePath(const FVector2D& EndPos)
{
	TArray<FVector2D> Path;

	if (!GameMode || !GameMode->GField)
	{
		UE_LOG(LogTemp, Warning, TEXT("CalculatePath: Invalid GameField."));
		return Path;
	}

	FVector2D StartPos = GetGridPosition();
	if (StartPos == EndPos)
	{
		return Path;
	}

	TQueue<FVector2D> Frontier;
	TSet<FVector2D> Visited;
	TMap<FVector2D, FVector2D> CameFrom;

	Frontier.Enqueue(StartPos);
	Visited.Add(StartPos);
	CameFrom.Add(StartPos, StartPos);

	TArray<FVector2D> Directions;
	Directions.Add(FVector2D(1, 0));
	Directions.Add(FVector2D(-1, 0));
	Directions.Add(FVector2D(0, 1));
	Directions.Add(FVector2D(0, -1));

	bool bFound = false;
	while (!Frontier.IsEmpty())
	{
		FVector2D Current;
		Frontier.Dequeue(Current);

		if (Current == EndPos)
		{
			bFound = true;
			break;
		}

		// Espandi i vicini
		for (auto& Dir : Directions)
		{
			FVector2D Neighbor = Current + Dir;

			bool bIsStartCell = (Neighbor == StartPos);
			if (!Visited.Contains(Neighbor) && IsValidGridCell(Neighbor, bIsStartCell))
			{
				Frontier.Enqueue(Neighbor);
				Visited.Add(Neighbor);
				CameFrom.Add(Neighbor, Current);
			}
		}
	}

	if (!bFound)
	{
		return Path;
	}

	FVector2D Tracer = EndPos;
	while (Tracer != StartPos)
	{
		Path.Add(Tracer);
		Tracer = CameFrom[Tracer];
	}
	Algo::Reverse(Path);

	return Path;
}


void AGameUnit::TakeDamageUnit(int32 DamageAmount, AGameUnit* Attacker)
{
	//UE_LOG(LogTemp, Warning, TEXT("TakeDamageUnit: ID=%d, Damage=%d"), GetGameUnitID(), DamageAmount);

	HitPoints -= DamageAmount;
	if (HitPoints < 0)
	{
		HitPoints = 0;
	}

	const int32 MaxHP = (GameUnitType == EGameUnitType::SNIPER) ? 20 : 40;

	const float HPPercent = (MaxHP > 0) ? (float)HitPoints / (float)MaxHP : 0.f;

	if (UUserWidget* BaseWidget = GameMode->UnitListWidget)
	{
		UUnitListWidget* MyWidget = Cast<UUnitListWidget>(BaseWidget);
		if (MyWidget)
		{
			MyWidget->UpdateUnitHealth((PlayerOwner == 0),
				GameUnitType,
				HPPercent);
		}
	}

	if (HitPoints <= 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Unit ID=%d is dead."), GetGameUnitID());

		if (AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode()))
		{
			if (GM->GField)
			{
				const FVector2D MyPos = GetGridPosition();
				if (ATile* Tile = GM->GField->TileMap.FindRef(MyPos))
				{
					Tile->SetTileStatus(-1, ETileStatus::EMPTY, nullptr);
				}
			}
		}
		Destroy();
		return;
	}
	if (Attacker && Attacker->GetGameUnitType() == EGameUnitType::SNIPER)
	{
		if (ASniper* TheSniper = Cast<ASniper>(Attacker))
		{
			TheSniper->HandleCounterAttack(this);
		}
	}
}

bool AGameUnit::IsDead() const
{
	return HitPoints <= 0;
}





