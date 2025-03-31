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
    PlayerNumber = 1;
    SequenceIndex = 0;
    bTurnEnded = false;
}

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

        if (Heuristic(Current.Pos, GoalPos) < Heuristic(BestNode.Pos, GoalPos))
        {
            BestNode = Current;
        }

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

    FVector2D CurrentPos = BestNode.Pos;
    Path.Add(CurrentPos);
    while (CameFrom.Contains(CurrentPos) && CurrentPos != StartPos)
    {
        CurrentPos = CameFrom[CurrentPos];
        Path.Insert(CurrentPos, 0);
    }
    return Path;
}

FVector2D AAStarPlayer::GetTargetPositionForUnit(AGameUnit* Unit, const FVector2D& EnemyPos)
{
    TArray<FVector2D> FullPath = AStarPathfinding(Unit, EnemyPos);
    if (FullPath.Num() == 0)
    {
        return Unit->GetGridPosition();
    }
    int32 MovementRange = Unit->GetMovementRange();
    int32 TargetIndex = FMath::Min(MovementRange, FullPath.Num() - 1);
    return FullPath[TargetIndex];
}


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


void AAStarPlayer::PerformAStarActionOnUnit(AGameUnit* Unit)
{
    if (!Unit)
    {
        return;
    }

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        return;
    }

    FVector2D UnitPos = Unit->GetGridPosition();
    FVector2D EnemyPos = GetClosestEnemyPosition(Unit);
    float ManhattanDistance = FMath::Abs(EnemyPos.X - UnitPos.X) + FMath::Abs(EnemyPos.Y - UnitPos.Y);

    if (ManhattanDistance == 1)
    {
        GM->GField->ShowLegalAttackOptionsForUnit(Unit);

        FTimerHandle AttackDelay;
        GetWorld()->GetTimerManager().SetTimer(AttackDelay, [this, Unit, GM]()
            {
                TArray<FVector2D> AttackOptions = Unit->CalculateAttackMoves();
                TArray<FVector2D> ValidAttacks;
                for (const FVector2D& APos : AttackOptions)
                {
                    if (GM->GField->IsValidPosition(APos))
                    {
                        ATile* Tile = GM->GField->TileMap.Contains(APos) ? GM->GField->TileMap[APos] : nullptr;
                        if (Tile && Tile->GetTileStatus() == ETileStatus::OCCUPIED)
                        {
                            AGameUnit* Target = Tile->GetGameUnit();
                            if (Target && Target->GetPlayerOwner() == 0)
                            {
                                ValidAttacks.Add(APos);
                            }
                        }
                    }
                }

                if (ValidAttacks.Num() > 0)
                {
                    FVector2D AttackChoice = ValidAttacks[FMath::RandRange(0, ValidAttacks.Num() - 1)];
                    GM->GField->AttackUnit(Unit, AttackChoice);
                    //UE_LOG(LogTemp, Warning, TEXT("A* AI -> Unit %d attacked enemy at (%f, %f) without moving"),Unit->GetGameUnitID(), AttackChoice.X, AttackChoice.Y);
                }
                Unit->bHasAttacked = true;
                GM->GField->ResetGameStatusField();
            }, 0.5f, false);

        return;
    }

    FVector2D TargetPos = GetTargetPositionForUnit(Unit, EnemyPos);
    GM->GField->MoveUnit(Unit, TargetPos, [this, Unit, GM, TargetPos]()
        {
            //UE_LOG(LogTemp, Warning, TEXT("A* AI -> Unit %d moved to (%f, %f)"), Unit->GetGameUnitID(), TargetPos.X, TargetPos.Y);
            Unit->bHasMoved = true;

            GM->GField->ShowLegalAttackOptionsForUnit(Unit);

            FTimerHandle AttackDelay;
            GetWorld()->GetTimerManager().SetTimer(AttackDelay, [this, Unit, GM]()
                {
                    TArray<FVector2D> PostMoveAttacks = Unit->CalculateAttackMoves();
                    TArray<FVector2D> PostMoveValid;
                    for (const FVector2D& APos : PostMoveAttacks)
                    {
                        if (GM->GField->IsValidPosition(APos))
                        {
                            ATile* Tile = GM->GField->TileMap.Contains(APos) ? GM->GField->TileMap[APos] : nullptr;
                            if (Tile && Tile->GetTileStatus() == ETileStatus::OCCUPIED)
                            {
                                AGameUnit* Target = Tile->GetGameUnit();
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
                        //UE_LOG(LogTemp, Warning, TEXT("A* AI -> Unit %d attacked enemy at (%f, %f)"), Unit->GetGameUnitID(), AttackChoice.X, AttackChoice.Y);
                    }
                    Unit->bHasAttacked = true;
                    GM->GField->ResetGameStatusField();
                }, 0.5f, false);
        });
}


void AAStarPlayer::DoNextUnitAction()
{
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM)
    {
        return;
    }

    while (SequenceIndex < UnitsSequence.Num() && !IsValid(UnitsSequence[SequenceIndex]))
    {
        SequenceIndex++;
    }
    if (SequenceIndex >= UnitsSequence.Num())
    {
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

    GM->GField->ShowLegalMovesForUnit(CurrentUnit);

    FTimerHandle ShowTilesDelay;
    GetWorld()->GetTimerManager().SetTimer(ShowTilesDelay, [this, CurrentUnit, GM]()
        {
            PerformAStarActionOnUnit(CurrentUnit);
            FTimerHandle NextUnitDelay;
            GetWorld()->GetTimerManager().SetTimer(NextUnitDelay, [this, GM]()
                {
                    GM->GField->ResetGameStatusField();
                    SequenceIndex++;
                    DoNextUnitAction();
                }, 1.0f, false);
        }, 0.5f, false);
}
