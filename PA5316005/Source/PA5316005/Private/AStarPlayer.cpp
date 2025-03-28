#include "AStarPlayer.h"
#include "AWGameMode.h"
#include "GameField.h"
#include "AWGameInstance.h"
#include "GameUnit.h"
#include "Tile.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AAStarPlayer::AAStarPlayer()
{
    // Imposta il numero del player e inizializza gli indici e la flag di turno
    PlayerNumber = 1;
    SequenceIndex = 0;
    bTurnEnded = false;
}

//
// Questo metodo viene chiamato quando � il turno di questo player
//
void AAStarPlayer::OnTurn()
{
    UAWGameInstance* GI = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GI)
    {
        GI->SetTurnMessage(TEXT("AI Turn (A*)"));
    }

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        return;
    }

    // Costruisci la sequenza delle unit� AI (PlayerOwner == 1) che non hanno ancora agito
    UnitsSequence.Empty();
    for (auto& Pair : GM->GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;
        if (Unit && Unit->GetPlayerOwner() == PlayerNumber && !(Unit->bHasMoved && Unit->bHasAttacked))
        {
            UnitsSequence.Add(Unit);
        }
    }

    SequenceIndex = 0;
    bTurnEnded = false;
    DoNextUnitAction();
}

//
// Implementazione dell'algoritmo A* per calcolare il percorso da Unit->posizione iniziale a GoalPos.
// Vengono considerate solo celle che non sono ostacolo oppure che sono occupate dalla stessa unit� (utile per la partenza).
//
TArray<FVector2D> AAStarPlayer::AStarPathfinding(AGameUnit* Unit, const FVector2D& GoalPos)
{
    TArray<FVector2D> Path;
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        return Path;
    }
    auto Field = GM->GField;

    struct Node
    {
        FVector2D Pos;
        float Cost;
        float Priority;
    };

    TArray<Node> OpenSet;
    TMap<FVector2D, FVector2D> CameFrom;
    TMap<FVector2D, float> CostSoFar;

    auto Heuristic = [&](const FVector2D& A, const FVector2D& B) -> float
        {
            return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
        };

    FVector2D StartPos = Unit->GetGridPosition();
    OpenSet.Add({ StartPos, 0.f, Heuristic(StartPos, GoalPos) });
    CostSoFar.Add(StartPos, 0.f);

    Node BestNode = OpenSet[0];
    TArray<FVector2D> Directions = {
        FVector2D(1, 0),
        FVector2D(-1, 0),
        FVector2D(0, 1),
        FVector2D(0, -1)
    };

    while (OpenSet.Num() > 0)
    {
        // Seleziona il nodo con la priorit� pi� bassa
        int32 BestIndex = 0;
        for (int32 i = 1; i < OpenSet.Num(); i++)
        {
            if (OpenSet[i].Priority < OpenSet[BestIndex].Priority)
            {
                BestIndex = i;
            }
        }
        Node Current = OpenSet[BestIndex];
        OpenSet.RemoveAt(BestIndex);

        // Aggiorna il best node se il nodo corrente � pi� vicino al goal
        if (Heuristic(Current.Pos, GoalPos) < Heuristic(BestNode.Pos, GoalPos))
        {
            BestNode = Current;
        }

        // Se abbiamo raggiunto l'obiettivo, ricostruisci il percorso
        if (Current.Pos == GoalPos)
        {
            FVector2D CurrentPos = GoalPos;
            Path.Add(CurrentPos);
            while (CameFrom.Contains(CurrentPos))
            {
                CurrentPos = CameFrom[CurrentPos];
                Path.Insert(CurrentPos, 0);
            }
            return Path;
        }

        // Esplora le 4 direzioni ortogonali
        for (const FVector2D& Dir : Directions)
        {
            FVector2D Next = Current.Pos + Dir;
            if (Field->IsValidPosition(Next) && Field->TileMap.Contains(Next))
            {
                ATile* NeighborTile = Field->TileMap[Next];
                if (NeighborTile)
                {
                    ETileStatus TileStatus = NeighborTile->GetTileStatus();
                    AGameUnit* Occupant = NeighborTile->GetGameUnit();

                    bool bIsMyOwnTile = (TileStatus == ETileStatus::OCCUPIED && Occupant == Unit);
                    bool bIsEmpty = (TileStatus == ETileStatus::EMPTY);

                    // Consenti il passaggio se la cella non � un ostacolo e se � vuota oppure occupata dalla stessa unit� (utile per la partenza)
                    if (TileStatus != ETileStatus::OBSTACLE && (bIsEmpty || bIsMyOwnTile))
                    {
                        float NewCost = CostSoFar[Current.Pos] + 1.f;
                        if (!CostSoFar.Contains(Next) || NewCost < CostSoFar[Next])
                        {
                            CostSoFar.Add(Next, NewCost);
                            float Priority = NewCost + Heuristic(Next, GoalPos);
                            OpenSet.Add({ Next, NewCost, Priority });
                            CameFrom.Add(Next, Current.Pos);
                        }
                    }
                }
            }
        }
    }

    // Se il goal non � raggiunto, restituisci il percorso dal "migliore" nodo trovato
    FVector2D CurrentPos = BestNode.Pos;
    Path.Add(CurrentPos);
    while (CameFrom.Contains(CurrentPos) && CurrentPos != StartPos)
    {
        CurrentPos = CameFrom[CurrentPos];
        Path.Insert(CurrentPos, 0);
    }
    return Path;
}

//
// Restituisce la cella target lungo il percorso che l'unit� pu� raggiungere entro il suo range di movimento
//
FVector2D AAStarPlayer::GetTargetPositionForUnit(AGameUnit* Unit, const FVector2D& EnemyPos)
{
    TArray<FVector2D> FullPath = AStarPathfinding(Unit, EnemyPos);
    if (FullPath.Num() == 0)
    {
        return Unit->GetGridPosition();
    }
    int32 MovementRange = Unit->GetMovementRange();
    // Se il percorso � pi� corto del range, il target � l'ultima cella
    int32 TargetIndex = FMath::Min(MovementRange, FullPath.Num() - 1);
    return FullPath[TargetIndex];
}

//
// Trova la posizione dell'unit� nemica pi� vicina (distanza Manhattan)
//
FVector2D AAStarPlayer::GetClosestEnemyPosition(AGameUnit* Unit)
{
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        return Unit->GetGridPosition();
    }
    FVector2D UnitPos = Unit->GetGridPosition();
    float BestDist = TNumericLimits<float>::Max();
    FVector2D ClosestEnemyPos = UnitPos;

    for (auto& Pair : GM->GField->GameUnitMap)
    {
        AGameUnit* OtherUnit = Pair.Value;
        // Considera solo le unit� nemiche (supponendo che l'avversario abbia PlayerOwner == 0)
        if (OtherUnit && OtherUnit->GetPlayerOwner() != Unit->GetPlayerOwner())
        {
            float Dist = FMath::Abs(OtherUnit->GetGridPosition().X - UnitPos.X) +
                FMath::Abs(OtherUnit->GetGridPosition().Y - UnitPos.Y);
            if (Dist < BestDist)
            {
                BestDist = Dist;
                ClosestEnemyPos = OtherUnit->GetGridPosition();
            }
        }
    }
    return ClosestEnemyPos;
}

//
// Esegue l'azione per l'unit�: usa l'algoritmo A* per calcolare il percorso verso
// l'unit� nemica pi� vicina e si sposta fino al limite del range di movimento.
// Dopo lo spostamento, se l'unit� nemica � nel range di attacco, mostra le tile di attacco
// e, dopo un breve ritardo, esegue l'attacco.
//
void AAStarPlayer::PerformAStarActionOnUnit(AGameUnit* Unit)
{
    if (!Unit)
    {
        return;
    }

    // Trova la posizione dell'unit� nemica pi� vicina
    FVector2D EnemyPos = GetClosestEnemyPosition(Unit);
    // Calcola la cella target raggiungibile entro il range di movimento dell'unit�
    FVector2D TargetPos = GetTargetPositionForUnit(Unit, EnemyPos);

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        return;
    }

    // Esegui lo spostamento usando il percorso A*
    GM->GField->MoveUnit(Unit, TargetPos, [this, Unit, GM, TargetPos]()
        {
            UE_LOG(LogTemp, Warning, TEXT("A* AI -> Unit %d moved to (%f, %f)"), Unit->GetGameUnitID(), TargetPos.X, TargetPos.Y);
            Unit->bHasMoved = true;

            // Dopo lo spostamento, mostra le tile di attacco (stessa logica degli altri player)
            GM->GField->ShowLegalAttackOptionsForUnit(Unit);

            // Attendi 0.5 secondi per visualizzare le tile di attacco, quindi esegui l'attacco se possibile
            FTimerHandle AttackDelay;
            GetWorld()->GetTimerManager().SetTimer(AttackDelay, [this, Unit, GM]()
                {
                    // Calcola le possibili celle di attacco dalla nuova posizione
                    TArray<FVector2D> PostMoveAttacks = Unit->CalculateAttackMoves();
                    TArray<FVector2D> PostMoveValid;
                    for (const FVector2D& APos : PostMoveAttacks)
                    {
                        if (GM->GField->IsValidPosition(APos))
                        {
                            ATile* T = GM->GField->TileMap[APos];
                            if (T && T->GetTileStatus() == ETileStatus::OCCUPIED)
                            {
                                AGameUnit* Target = T->GetGameUnit();
                                // Se il bersaglio � nemico (supponendo che nemico abbia PlayerOwner == 0)
                                if (Target && Target->GetPlayerOwner() == 0)
                                {
                                    PostMoveValid.Add(APos);
                                }
                            }
                        }
                    }

                    if (PostMoveValid.Num() > 0)
                    {
                        FVector2D AttackChoice = PostMoveValid[FMath::RandRange(0, PostMoveValid.Num() - 1)];
                        GM->GField->AttackUnit(Unit, AttackChoice);
                        UE_LOG(LogTemp, Warning, TEXT("A* AI -> Unit %d attacked enemy at (%f, %f)"), Unit->GetGameUnitID(), AttackChoice.X, AttackChoice.Y);
                    }
                    Unit->bHasAttacked = true;
                    // Resetta le evidenziazioni
                    GM->GField->ResetGameStatusField();
                }, 0.5f, false);
        });
}

//
// Gestisce la sequenza delle azioni delle unit� in questo player AI.
// Per ogni unit�, mostra le tile di movimento, attende un breve ritardo, esegue l'azione A* (movimento e potenzialmente attacco)
// e segna l'unit� come avente eseguito movimento ed attacco.
// Quando tutte le unit� hanno agito, imposta bTurnEnded e passa il turno.
//
void AAStarPlayer::DoNextUnitAction()
{
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM)
    {
        return;
    }

    // Salta le unit� non valide
    while (SequenceIndex < UnitsSequence.Num() && !IsValid(UnitsSequence[SequenceIndex]))
    {
        SequenceIndex++;
    }
    if (SequenceIndex >= UnitsSequence.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("A* AI: Tutte le unit� hanno completato le azioni."));
        bTurnEnded = true;
        GM->NextTurn();
        return;
    }

    AGameUnit* CurrentUnit = UnitsSequence[SequenceIndex];
    if (!CurrentUnit)
    {
        SequenceIndex++;
        DoNextUnitAction();
        return;
    }

    // Mostra le tile di movimento per l'unit� corrente
    UE_LOG(LogTemp, Warning, TEXT("A* AI -> Mostro le tile per l'unit� ID=%d"), CurrentUnit->GetGameUnitID());
    GM->GField->ShowLegalMovesForUnit(CurrentUnit);

    // Attendi 0.5 secondi per visualizzare le tile, quindi esegui l'azione A* (movimento e attacco)
    FTimerHandle ShowTilesDelay;
    GetWorld()->GetTimerManager().SetTimer(ShowTilesDelay, [this, CurrentUnit, GM]()
        {
            PerformAStarActionOnUnit(CurrentUnit);

            // Dopo un ulteriore ritardo (1 secondo), resetta le tile e passa all'unit� successiva
            FTimerHandle NextUnitDelay;
            GetWorld()->GetTimerManager().SetTimer(NextUnitDelay, [this, GM]()
                {
                    GM->GField->ResetGameStatusField();
                    SequenceIndex++;
                    DoNextUnitAction();
                }, 1.0f, false);
        }, 0.5f, false);
}
