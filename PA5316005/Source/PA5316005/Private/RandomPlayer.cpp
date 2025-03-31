#include "RandomPlayer.h"
#include "AWGameMode.h"
#include "GameField.h"
#include "GameUnit.h"
#include "AWGameInstance.h"
#include "Kismet/GameplayStatics.h"

#include "AWGameInstance.h"
#include "EngineUtils.h"

ARandomPlayer::ARandomPlayer()
{
    PlayerNumber = 1;
    SequenceIndex = 0;

}

void ARandomPlayer::OnTurn()
{
    if (!GetWorld())
    {
        //UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GetWorld() is nullptr!"));
        return;
    }

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM)
    {
        //UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GameMode is nullptr!"));
        return;
    }

    if (!GM->GField)
    {
        //UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GField is nullptr!"));
        return;
    }

  
    UAWGameInstance* GI = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GI)
    {
        GI->SetTurnMessage(TEXT("AI Turn"));
    }
    

 
    TArray<AGameUnit*> AIUnits;
    for (auto& Pair : GM->GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;
        if (Unit && Unit->GetPlayerOwner() == 1 && !(Unit->bHasMoved && Unit->bHasAttacked))
        {
            AIUnits.Add(Unit);
        }
    }



    SequenceIndex = 0;
    UnitsSequence = AIUnits;
    DoNextUnitAction();
}

void ARandomPlayer::DoNextUnitAction()
{
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM)
    {
        return;
    }

    // Se abbiamo finito di processare le unità, passa il turno
    while (SequenceIndex < UnitsSequence.Num() && !IsValid(UnitsSequence[SequenceIndex]))
    {
        SequenceIndex++;
    }
    if (SequenceIndex >= UnitsSequence.Num())
    {
       // UE_LOG(LogTemp, Log, TEXT("AI: Tutte le unità hanno completato le azioni. Passo il turno."));
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

   
    //UE_LOG(LogTemp, Warning, TEXT("AI -> Mostro le tile per l'unità ID=%d"), CurrentUnit->GetGameUnitID());
    GM->GField->ShowLegalMovesForUnit(CurrentUnit);

   
    FTimerHandle ShowTilesDelay;
    GetWorldTimerManager().SetTimer(ShowTilesDelay, [this, CurrentUnit, GM]()
        {
            PerformRandomActionOnUnit(CurrentUnit);
            CurrentUnit->bHasMoved = true;
            CurrentUnit->bHasAttacked = true;

            FTimerHandle NextUnitDelay;
            GetWorldTimerManager().SetTimer(NextUnitDelay, [this, GM]()
                {
                    GM->GField->ResetGameStatusField();
                    SequenceIndex++;
                    DoNextUnitAction();
                }, 1.0f, false);
        }, 2.0f, false);
}


void ARandomPlayer::PerformRandomActionOnUnit(AGameUnit* Unit)
{
    if (!Unit) return;

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField) return;

    TArray<FVector2D> Moves = Unit->CalculateLegalMoves();
    TArray<FVector2D> ValidMoves;
    for (const FVector2D& MovePos : Moves)
    {
        if (GM->GField->IsValidPosition(MovePos))
        {
            ATile* Tile = GM->GField->TileMap[MovePos];
            if (Tile && Tile->GetTileStatus() == ETileStatus::EMPTY)
            {
                ValidMoves.Add(MovePos);
            }
        }
    }
    bool bCanMove = (ValidMoves.Num() > 0);


    TArray<FVector2D> Attacks = Unit->CalculateAttackMoves();
    TArray<FVector2D> ValidAttacks;
    for (const FVector2D& AttackPos : Attacks)
    {
        if (GM->GField->IsValidPosition(AttackPos))
        {
            ATile* Tile = GM->GField->TileMap[AttackPos];
            if (Tile && Tile->GetTileStatus() == ETileStatus::OCCUPIED)
            {
                AGameUnit* Target = Tile->GetGameUnit();
                if (Target && Target->GetPlayerOwner() == 0)
                {
                    ValidAttacks.Add(AttackPos);
                }
            }
        }
    }
    bool bCanAttack = (ValidAttacks.Num() > 0);

    if (bCanMove && bCanAttack)
    {
        //UE_LOG(LogTemp, Log, TEXT("AI -> Unità ID %d: Muovi e poi Attacca"), Unit->GetGameUnitID());
        FVector2D MoveChoice = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];

        GM->GField->MoveUnit(Unit, MoveChoice, [this, Unit, GM]()
            {
                GM->GField->ResetGameStatusField();

                GM->GField->ShowLegalAttackOptionsForUnit(Unit);

                FTimerHandle AttackDelay;
                GetWorld()->GetTimerManager().SetTimer(
                    AttackDelay,
                    [this, Unit, GM]()
                    {
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
                        }

                        GM->GField->ResetGameStatusField();
                    },
                    0.5f,
                    false
                );
            });
    }
    else if (bCanAttack)
    {
        //UE_LOG(LogTemp, Log, TEXT("AI -> Unità ID %d: Solo Attacca"), Unit->GetGameUnitID());
        FVector2D AttackChoice = ValidAttacks[FMath::RandRange(0, ValidAttacks.Num() - 1)];
        GM->GField->AttackUnit(Unit, AttackChoice);

        GM->GField->ResetGameStatusField();
    }
    else if (bCanMove)
    {
        //UE_LOG(LogTemp, Log, TEXT("AI -> Unità ID %d: Solo Muovi"), Unit->GetGameUnitID());
        FVector2D MoveChoice = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];

        GM->GField->MoveUnit(Unit, MoveChoice, [GM]()
            {
                GM->GField->ResetGameStatusField();
            });
    }
    else
    {
        //UE_LOG(LogTemp, Warning, TEXT("AI -> Unità ID %d non può né muoversi né attaccare."), Unit->GetGameUnitID());
    }
}


