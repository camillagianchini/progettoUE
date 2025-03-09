#include "RandomPlayer.h"
#include "GameField.h"
#include "Tile.h"
#include "AWPlayerController.h"
#include "Sniper.h"
#include "Brawler.h"
#include "Kismet/GameplayStatics.h"

void ARandomPlayer::OnTurn()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Turno della IA (Random)"));
    DecideMove();
}

void ARandomPlayer::DecideMove()
{
    // Trova tutte le unità dell'IA
    TArray<AActor*> AIUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AIUnits);

    TArray<ASniper*> AI_Snipers;
    TArray<ABrawler*> AI_Brawlers;

    for (AActor* Unit : AIUnits)
    {
        if (Unit->ActorHasTag("AIUnit"))
        {
            if (ASniper* Sniper = Cast<ASniper>(Unit))
            {
                AI_Snipers.Add(Sniper);
            }
            else if (ABrawler* Brawler = Cast<ABrawler>(Unit))
            {
                AI_Brawlers.Add(Brawler);
            }
        }
    }

    // Muove e attacca con gli Sniper
    for (ASniper* Sniper : AI_Snipers)
    {
        MoveAndAttack(Sniper);
    }

    // Muove e attacca con i Brawler
    for (ABrawler* Brawler : AI_Brawlers)
    {
        MoveAndAttack(Brawler);
    }

    // Dopo che entrambe le unità hanno agito, passa il turno all'umano
    AAWGameMode* GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode)
    {
        GameMode->EndTurn();
    }
}

void ARandomPlayer::MoveAndAttack(ASniper* Sniper)
{
    FVector2D CurrentPos = FVector2D(Sniper->GetActorLocation().X, Sniper->GetActorLocation().Y);
    FVector2D TargetPos = FVector2D(GameMode->FindClosestEnemy(Sniper).X, GameMode->FindClosestEnemy(Sniper).Y);

    // Se il nemico è nel range di attacco, attacca
    if (GameMode->IsInAttackRange(Sniper, TargetPos))
    {
        GameMode->Attack(Sniper, TargetPos);
        return;
    }

    // Se non può attaccare subito, si muove usando A*
    TArray<FVector2D> Path = GetWorld()->GetFirstPlayerController<AAWPlayerController>()->FindPathAStar(CurrentPos, TargetPos);

    if (Path.Num() > 1)
    {
        FVector2D NextStep = Path[1];
        FVector NewLocation = FVector(NextStep.X, NextStep.Y, Sniper->GetActorLocation().Z);
        Sniper->SetActorLocation(NewLocation);
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("Sniper si è mosso"));

        // Dopo il movimento, se ora è nel range di attacco, attacca
        if (GameMode->IsInAttackRange(Sniper, TargetPos))
        {
            GameMode->Attack(Sniper, TargetPos);
        }
    }
}


void ARandomPlayer::MoveAndAttack(ABrawler* Brawler)
{
    FVector2D CurrentPos = FVector2D(Brawler->GetActorLocation().X, Brawler->GetActorLocation().Y);
    FVector2D TargetPos = FVector2D(GameMode->FindClosestEnemy(Brawler).X, GameMode->FindClosestEnemy(Brawler).Y);

    // Se il nemico è nel range di attacco, attacca
    if (GameMode->IsInAttackRange(Brawler, TargetPos))
    {
        GameMode->Attack(Brawler, TargetPos);
        return;
    }

    // Se non può attaccare subito, si muove usando A*
    TArray<FVector2D> Path = GetWorld()->GetFirstPlayerController<AAWPlayerController>()->FindPathAStar(CurrentPos, TargetPos);

    if (Path.Num() > 1)
    {
        FVector2D NextStep = Path[1];
        FVector NewLocation = FVector(NextStep.X, NextStep.Y, Brawler->GetActorLocation().Z);
        Brawler->SetActorLocation(NewLocation);
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("Brawler si è mosso"));

        // Dopo il movimento, se ora è nel range di attacco, attacca
        if (GameMode->IsInAttackRange(Brawler, TargetPos))
        {
            GameMode->Attack(Brawler, TargetPos);
        }
    }
}

