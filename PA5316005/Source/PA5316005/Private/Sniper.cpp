#include "Sniper.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

ASniper::ASniper()
{
	PrimaryActorTick.bCanEverTick = true;

	// Inizializzazione delle variabili in base alle specifiche
	MovementRange = 3;
	AttackRange = 10;
	MinDamage = 4;
	MaxDamage = 8;
	Health = 20;
}

void ASniper::BeginPlay()
{
	Super::BeginPlay();
}

void ASniper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Funzione per muoversi sulla griglia
void ASniper::MoveTo(FVector2D NewPosition)
{
	// Controllo se il movimento è entro il range
	if (FMath::Abs(NewPosition.X - GetActorLocation().X) + FMath::Abs(NewPosition.Y - GetActorLocation().Y) <= MovementRange)
	{
		SetActorLocation(FVector(NewPosition.X, NewPosition.Y, GetActorLocation().Z));
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Sniper si è mosso"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Movimento fuori range!"));
	}
}

// Funzione di attacco
void ASniper::Attack(AActor* Target)
{
	if (!Target)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Nessun bersaglio valido!"));
		return;
	}

	if (!IsTargetInRange(Target))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Bersaglio fuori dal range di attacco!"));
		return;
	}

	// Calcolo danno casuale
	int32 Damage = FMath::RandRange(MinDamage, MaxDamage);

	// Applica danno all'unità bersaglio
	ASniper* EnemyUnit = Cast<ASniper>(Target);
	if (EnemyUnit)
	{
		EnemyUnit->TakeDamage(Damage);
	}

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("Sniper infligge %d danni!"), Damage));

	// Controllo per il contrattacco
	CounterAttack(Target);
}

// Funzione per verificare se il bersaglio è nel range d'attacco
bool ASniper::IsTargetInRange(AActor* Target)
{
	// Ottiene la distanza dalla posizione attuale
	FVector TargetLocation = Target->GetActorLocation();
	FVector CurrentLocation = GetActorLocation();

	// Distanza in griglia Manhattan (orizzontale + verticale)
	int32 Distance = FMath::Abs(TargetLocation.X - CurrentLocation.X) + FMath::Abs(TargetLocation.Y - CurrentLocation.Y);

	return Distance <= AttackRange;
}

// Funzione per ricevere danno
void ASniper::TakeDamage(int32 Amount)
{
	Health -= Amount;

	if (Health <= 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Sniper è stato eliminato!"));
		Destroy();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Sniper subisce %d danni! Vita rimanente: %d"), Amount, Health));
	}
}

// Controllo per il contrattacco
void ASniper::CounterAttack(AActor* Attacker)
{
	// Se l'attaccante è vicino (range 1) oppure è uno Sniper, contrattacca
	FVector AttackerLocation = Attacker->GetActorLocation();
	FVector CurrentLocation = GetActorLocation();

	int32 Distance = FMath::Abs(AttackerLocation.X - CurrentLocation.X) + FMath::Abs(AttackerLocation.Y - CurrentLocation.Y);

	if (Distance == 1 || Cast<ASniper>(Attacker))
	{
		int32 CounterDamage = FMath::RandRange(1, 3);
		TakeDamage(CounterDamage);

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, FString::Printf(TEXT("Sniper riceve %d danni da contrattacco!"), CounterDamage));
	}
}