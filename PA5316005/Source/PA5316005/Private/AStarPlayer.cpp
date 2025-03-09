#include "AStarPlayer.h"
#include "AWGameMode.h"
#include "AWPlayerController.h"
#include "Sniper.h"
#include "Brawler.h"
#include "Kismet/GameplayStatics.h"

AAStarPlayer::AAStarPlayer()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAStarPlayer::OnTurn()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Turno della IA (AStar)"));
    DecideMove();  // Usa la logica di RandomPlayer, ma con A*
}

void AAStarPlayer::MoveAndAttack(ASniper* Sniper)
{
    FVector2D CurrentPos = FVector2D(Sniper->GetActorLocation().X, Sniper->GetActorLocation().Y);
    FVector2D TargetPos = FVector2D(GameMode->FindClosestEnemy(Sniper).X, GameMode->FindClosestEnemy(Sniper).Y);

    // Trova il percorso con A*
    TArray<FVector2D> Path = GetWorld()->GetFirstPlayerController<AAWPlayerController>()->FindPathAStar(CurrentPos, TargetPos);

    if (Path.Num() > 1)
    {
        FVector2D NextStep = Path[1];
        FVector NewLocation = FVector(NextStep.X, NextStep.Y, Sniper->GetActorLocation().Z);
        Sniper->SetActorLocation(NewLocation);
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("AStar Sniper si è mosso con A*"));

        if (GameMode->IsInAttackRange(Sniper, TargetPos))
        {
            GameMode->Attack(Sniper, TargetPos);
        }
    }
}

void AAStarPlayer::MoveAndAttack(ABrawler* Brawler)
{
    FVector2D CurrentPos = FVector2D(Brawler->GetActorLocation().X, Brawler->GetActorLocation().Y);
    FVector2D TargetPos = FVector2D(GameMode->FindClosestEnemy(Brawler).X, GameMode->FindClosestEnemy(Brawler).Y);

    // Trova il percorso con A*
    TArray<FVector2D> Path = GetWorld()->GetFirstPlayerController<AAWPlayerController>()->FindPathAStar(CurrentPos, TargetPos);

    if (Path.Num() > 1)
    {
        FVector2D NextStep = Path[1];
        FVector NewLocation = FVector(NextStep.X, NextStep.Y, Brawler->GetActorLocation().Z);
        Brawler->SetActorLocation(NewLocation);
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("AStar Brawler si è mosso con A*"));

        if (GameMode->IsInAttackRange(Brawler, TargetPos))
        {
            GameMode->Attack(Brawler, TargetPos);
        }
    }
}

