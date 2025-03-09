#include "AWPlayerController.h"
#include "Tile.h"
#include "GameField.h"
#include "Sniper.h"
#include "Brawler.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"

AAWPlayerController::AAWPlayerController()
{
    bShowMouseCursor = true;  // Mostra il cursore del mouse
}

void AAWPlayerController::BeginPlay()
{
    Super::BeginPlay();
}

void AAWPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();  // Ora correttamente chiamato dalla classe APlayerController

    InputComponent->BindAction("LeftClick", IE_Pressed, this, &AAWPlayerController::HandleClick);
}


void AAWPlayerController::HandleClick()
{
    FHitResult Hit;
    GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (Hit.bBlockingHit)
    {
        AActor* ClickedActor = Hit.GetActor();

        // Se clicchiamo su un'unità, la selezioniamo
        if (ClickedActor->ActorHasTag("HumanUnit"))
        {
            SelectUnit(ClickedActor);
        }
        // Se clicchiamo su una cella, proviamo a muovere o attaccare
        else if (ATile* Tile = Cast<ATile>(ClickedActor))
        {
            if (SelectedUnit)
            {
                MoveSelectedUnit(FVector2D(Tile->GetActorLocation().X, Tile->GetActorLocation().Y));
            }
        }
        // Se clicchiamo su un nemico, attacchiamo
        else if (ClickedActor->ActorHasTag("AIUnit"))
        {
            AttackWithSelectedUnit(ClickedActor);
        }
    }
}

void AAWPlayerController::SelectUnit(AActor* NewSelectedUnit)
{
    SelectedUnit = NewSelectedUnit;
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Unità selezionata"));
}

void AAWPlayerController::MoveSelectedUnit(FVector2D TargetPosition)
{
    if (!SelectedUnit) return;

    FVector2D StartPos = FVector2D(SelectedUnit->GetActorLocation().X, SelectedUnit->GetActorLocation().Y);

    // Determina il range massimo di movimento in base all'unità selezionata
    int32 MovementRange = 0;
    if (ASniper* Sniper = Cast<ASniper>(SelectedUnit))
    {
        MovementRange = Sniper->MovementRange;
    }
    else if (ABrawler* Brawler = Cast<ABrawler>(SelectedUnit))
    {
        MovementRange = Brawler->MovementRange;
    }

    // Controlla se la cella è nel range di movimento
    if (!IsMoveValid(StartPos, TargetPosition, MovementRange))
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Movimento fuori range!"));
        return;
    }

    // Se il movimento è valido, aggiorna la posizione
    FVector NewLocation = FVector(TargetPosition.X, TargetPosition.Y, SelectedUnit->GetActorLocation().Z);
    SelectedUnit->SetActorLocation(NewLocation);
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Unità spostata"));
}

bool AAWPlayerController::IsMoveValid(FVector2D Start, FVector2D Target, int32 MovementRange)
{
    int32 Distance = FMath::Abs(Target.X - Start.X) + FMath::Abs(Target.Y - Start.Y); // Distanza Manhattan
    return (Distance <= MovementRange);
}


void AAWPlayerController::AttackWithSelectedUnit(AActor* Target)
{
    if (!SelectedUnit || !Target) return;

    if (ASniper* Sniper = Cast<ASniper>(SelectedUnit))
    {
        Sniper->Attack(Target);
    }
    else if (ABrawler* Brawler = Cast<ABrawler>(SelectedUnit))
    {
        Brawler->Attack(Target);
    }

    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Attacco eseguito"));
}

TArray<FVector2D> AAWPlayerController::FindPathAStar(FVector2D Start, FVector2D Target)
{
    TArray<FVector2D> Path;

    // Strutture dati per A*
    TMap<FVector2D, FVector2D> CameFrom;
    TMap<FVector2D, float> GScore;
    TMap<FVector2D, float> FScore;
    TPriorityQueue<FVector2D, TArray<FVector2D>, FScoreComparator> OpenSet;

    GScore.Add(Start, 0);
    FScore.Add(Start, FVector2D::Distance(Start, Target));
    OpenSet.Enqueue(Start);

    while (!OpenSet.IsEmpty())
    {
        FVector2D Current;
        OpenSet.Dequeue(Current);

        if (Current == Target)
        {
            while (CameFrom.Contains(Current))
            {
                Path.Insert(Current, 0);
                Current = CameFrom[Current];
            }
            Path.Insert(Start, 0);
            return Path;
        }

        TArray<FVector2D> Neighbors = {
            FVector2D(Current.X + 1, Current.Y),
            FVector2D(Current.X - 1, Current.Y),
            FVector2D(Current.X, Current.Y + 1),
            FVector2D(Current.X, Current.Y - 1)
        };

        for (FVector2D Neighbor : Neighbors)
        {
            if (!GameField->IsValidTile(Neighbor)) continue;
            if (GameField->IsObstacle(Neighbor)) continue;

            float TentativeGScore = GScore[Current] + 1;
            if (!GScore.Contains(Neighbor) || TentativeGScore < GScore[Neighbor])
            {
                CameFrom.Add(Neighbor, Current);
                GScore.Add(Neighbor, TentativeGScore);
                FScore.Add(Neighbor, TentativeGScore + FVector2D::Distance(Neighbor, Target));
                OpenSet.Enqueue(Neighbor);
            }
        }
    }

    return Path;
}
