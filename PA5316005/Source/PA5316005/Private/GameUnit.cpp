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
	bHasMoved = false;
	bHasAttacked = false;
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

TArray<FVector2D> AGameUnit::CalculateAttackMoves() const
{
	return TArray<FVector2D>();
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
	if (!GameMode || !GameMode->GField)
		return Result;
	
	FVector2D StartPos = GetGridPosition();

	//UE_LOG(LogTemp, Warning, TEXT("CalculateLegalMoves chiamato per unità ID=%d, StartPos=(%.0f, %.0f), MovementRange=%d"),
		//tGameUnitID(), StartPos.X, StartPos.Y, MovementRange);


	// Strutture BFS
	TQueue<FVector2D> Frontier;
	TSet<FVector2D> Visited;
	TMap<FVector2D, int32> DistMap;


	// Inserisci la cella di partenza manualmente, senza controllo
	Frontier.Enqueue(StartPos);
	Visited.Add(StartPos);
	DistMap.Add(StartPos, 0);

	// Direzioni verticali/orizzontali
	static TArray<FVector2D> Dirs = {
		FVector2D(1, 0), FVector2D(-1, 0),
		FVector2D(0, 1), FVector2D(0, -1)
	};

	while (!Frontier.IsEmpty())
	{
		FVector2D Current;
		Frontier.Dequeue(Current);
		//UE_LOG(LogTemp, Warning, TEXT("BFS Current = (%.0f, %.0f), Dist = %d"),
			//Current.X, Current.Y, DistMap[Current]);
		int32 CurrentDist = DistMap[Current];

		// Espandi nei 4 vicini
		for (auto& Dir : Dirs)
		{
			FVector2D Next = Current + Dir;
			int32 NextDist = CurrentDist + 1;
			if (NextDist <= MovementRange && !Visited.Contains(Next))
			{
				// Imposta il flag bIsStart: sarà true solo se Next è la cella di partenza
				bool bIsStart = (Next == StartPos);
				if (IsValidGridCell(Next, bIsStart))
				{
					// Aggiungi Next alle mosse se non è la cella di partenza
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
	//UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell(%.0f, %.0f), bIsStart=%s"),
		//CellPos.X, CellPos.Y, bIsStart ? TEXT("true") : TEXT("false"));

	if (!GameMode || !GameMode->GField)
	{
		UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell: GameMode o GField null per cella %s"), *CellPos.ToString());
		return false;
	}

	AGameField* GF = GameMode->GField;
	if (!GF->TileMap.Contains(CellPos))
	{
		UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell: Cella %s non trovata nella TileMap"), *CellPos.ToString());
		return false;
	}

	
		ATile* Tile = GF->TileMap[CellPos];
		if (!Tile)
		{
			//UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell: Tile a %s è NULL"), *CellPos.ToString());
			return false;
		}

		if (bIsStart)
		{
			if (Tile->GetGameUnit() == this)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cella di partenza %s valida: occupata dalla stessa unità (ID: %d)"), *CellPos.ToString(), this->GetGameUnitID());
				return true;
			}
		}


		// Controlla se la tile è un ostacolo
		if (Tile->GetTileStatus() == ETileStatus::OBSTACLE)
		{
			//UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell: Cella %s scartata -> OBSTACLE"), *CellPos.ToString());
			return false;
		}

		// Se la tile è OCCUPIED...
		if (Tile->GetTileStatus() == ETileStatus::OCCUPIED)
		{
			if (bIsStart)
			{
				if (Tile->GetGameUnit() == this)
				{
					//UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell: Cella %s (start) valida: occupata da me stesso"), *CellPos.ToString());
					return true;
				}
	
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell: Cella %s scartata -> OCCUPIED"), *CellPos.ToString());
				return false;
			}
		}

		// Se la tile è EMPTY, è valida
		//UE_LOG(LogTemp, Warning, TEXT("IsValidGridCell: Cella %s valida: EMPTY"), *CellPos.ToString());
		return true;
	}


TArray<FVector2D> AGameUnit::CalculatePath(const FVector2D& EndPos)
{
	TArray<FVector2D> Path;

	// 1) Recupera il riferimento al GameField
	//    (assumendo di avere GameMode->GField disponibile)
	if (!GameMode || !GameMode->GField)
	{
		UE_LOG(LogTemp, Warning, TEXT("CalculatePath: GameField non valido."));
		return Path;
	}

	FVector2D StartPos = GetGridPosition();
	if (StartPos == EndPos)
	{
		// Se già stai sulla cella di destinazione, path vuoto
		return Path;
	}

	// 2) Strutture BFS
	TQueue<FVector2D> Frontier;
	TSet<FVector2D> Visited;
	TMap<FVector2D, FVector2D> CameFrom;

	Frontier.Enqueue(StartPos);
	Visited.Add(StartPos);
	CameFrom.Add(StartPos, StartPos);

	// 3) Direzioni (4 direzioni ortogonali)
	TArray<FVector2D> Directions;
	Directions.Add(FVector2D(1, 0));
	Directions.Add(FVector2D(-1, 0));
	Directions.Add(FVector2D(0, 1));
	Directions.Add(FVector2D(0, -1));

	// 4) BFS loop
	bool bFound = false;
	while (!Frontier.IsEmpty())
	{
		FVector2D Current;
		Frontier.Dequeue(Current);

		// Se abbiamo trovato EndPos, interrompiamo
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
		// Nessun path trovato
		return Path;
	}

	// 5) Ricostruisci il percorso facendo backtracking da EndPos
	FVector2D Tracer = EndPos;
	while (Tracer != StartPos)
	{
		Path.Add(Tracer);
		Tracer = CameFrom[Tracer];
	}
	// (opzionale) Aggiungi la cella di partenza se vuoi
	// Path.Add(StartPos);

	// Il path ora è al contrario (da EndPos a StartPos), quindi lo inverti
	Algo::Reverse(Path);

	return Path;
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





