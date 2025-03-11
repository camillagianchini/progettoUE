#include "GameUnit.h"
#include "GameField.h"
#include "AWGameMode.h"
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

// Called to bind functionality to input
void AGameUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
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

TArray<FVector2D> AGameUnit::CalculateLegalMoves() const
{
	TArray<FVector2D> LegalMoves;

	// Calcola le mosse possibili in orizzontale e verticale basate sul MovementRange
	for (int32 Offset = 1; Offset <= MovementRange; Offset++)
	{
		// Movimento a destra
		LegalMoves.Add(FVector2D(GameUnitGridPosition.X + Offset, GameUnitGridPosition.Y));
		// Movimento a sinistra
		LegalMoves.Add(FVector2D(GameUnitGridPosition.X - Offset, GameUnitGridPosition.Y));
		// Movimento verso l'alto
		LegalMoves.Add(FVector2D(GameUnitGridPosition.X, GameUnitGridPosition.Y + Offset));
		// Movimento verso il basso
		LegalMoves.Add(FVector2D(GameUnitGridPosition.X, GameUnitGridPosition.Y - Offset));
	}

	return LegalMoves;
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




