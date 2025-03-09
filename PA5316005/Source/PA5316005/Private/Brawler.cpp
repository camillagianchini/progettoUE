#include "Brawler.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

ABrawler::ABrawler()
{
    PrimaryActorTick.bCanEverTick = true;

    // Inizializzazione delle variabili in base alle specifiche
    MovementRange = 6;
    AttackRange = 1;
    MinDamage = 1;
    MaxDamage = 6;
    Health = 40;
}

void ABrawler::BeginPlay()
{
    Super::BeginPlay();
}

void ABrawler::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Funzione per muoversi sulla griglia
void ABrawler::MoveTo(FVector2D NewPosition)
{
    // Controllo se il movimento è entro il range
    if (FMath::Abs(NewPosition.X - GetActorLocation().X) + FMath::Abs(NewPosition.Y - GetActorLocation().Y) <= MovementRange)
    {
        SetActorLocation(FVector(NewPosition.X, NewPosition.Y, GetActorLocation().Z));
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Brawler si è mosso"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Movimento fuori range!"));
    }
}

// Funzione di attacco
void ABrawler::Attack(AActor* Target)
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
    ABrawler* EnemyUnit = Cast<ABrawler>(Target);
    if (EnemyUnit)
    {
        EnemyUnit->TakeDamage(Damage);
    }

    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("Brawler infligge %d danni!"), Damage));

}

// Funzione per verificare se il bersaglio è nel range d'attacco
bool ABrawler::IsTargetInRange(AActor* Target)
{
    // Ottiene la distanza dalla posizione attuale
    FVector TargetLocation = Target->GetActorLocation();
    FVector CurrentLocation = GetActorLocation();

    // Distanza in griglia Manhattan (orizzontale + verticale)
    int32 Distance = FMath::Abs(TargetLocation.X - CurrentLocation.X) + FMath::Abs(TargetLocation.Y - CurrentLocation.Y);

    return Distance <= AttackRange;
}

// Funzione per ricevere danno
void ABrawler::TakeDamage(int32 Amount)
{
    Health -= Amount;

    if (Health <= 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Brawler è stato eliminato!"));
        Destroy();
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Brawler subisce %d danni! Vita rimanente: %d"), Amount, Health));
    }
}

