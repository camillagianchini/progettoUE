#include "RandomPlayer.h"
#include "AWGameMode.h"
#include "GameField.h"
#include "GameUnit.h"
#include "EngineUtils.h"

ARandomPlayer::ARandomPlayer()
{
    // Differenzia il PlayerNumber dall'umano (0)
    PlayerNumber = 1;
    SequenceIndex = 0;
}

void ARandomPlayer::OnTurn()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GetWorld() � nullptr!"));
        return;
    }

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM)
    {
        UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GetAuthGameMode() � nullptr!"));
        return;
    }

    if (!GM->GField)
    {
        UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GField � nullptr!"));
        return;
    }

    // Filtra le unit� AI che non hanno completato (non hanno ancora effettuato movimento e attacco)
    TArray<AGameUnit*> AIUnits;
    for (auto& Pair : GM->GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;
        // Se almeno una delle azioni non � stata eseguita, includi la unit�
        if (Unit && Unit->GetPlayerOwner() == 1 && !(Unit->bHasMoved && Unit->bHasAttacked))
        {
            AIUnits.Add(Unit);
        }
    }

    if (AIUnits.Num() == 0)
    {
        // Se tutte le unit� hanno completato, passa il turno
        GM->NextTurn();
        return;
    }

    SequenceIndex = 0;
    UnitsSequence = AIUnits;
    DoNextUnitAction();
}

void ARandomPlayer::DoNextUnitAction()
{
    if (SequenceIndex >= UnitsSequence.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Tutte le unit� hanno completato le azioni. Passo il turno."));
        AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
        if (GM)
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

    UE_LOG(LogTemp, Warning, TEXT("Chiamato ShowLegalMovesForUnit per unit� ID=%d"), CurrentUnit->GetGameUnitID());
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (GM && GM->GField)
    {
        GM->GField->ShowLegalMovesForUnit(CurrentUnit);
    }

    // Attendi 2 secondi (per mostrare l'evidenziazione) e poi esegui l'azione
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this, CurrentUnit]()
        {
            PerformRandomActionOnUnit(CurrentUnit);
            // Indipendentemente dall'azione eseguita, segnala che la unit� ha "completato" il turno
            CurrentUnit->bHasMoved = true;
            CurrentUnit->bHasAttacked = true;
            SequenceIndex++;
            DoNextUnitAction();
        }, 2.0f, false);
}

void ARandomPlayer::PerformRandomActionOnUnit(AGameUnit* Unit)
{
    if (!Unit)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformRandomActionOnUnit: unit� nulla."));
        return;
    }

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformRandomActionOnUnit: GameMode o GameField non trovati."));
        return;
    }

    // Calcola le possibili mosse per il movimento
    TArray<FVector2D> Moves = Unit->CalculateLegalMoves();
    TArray<FVector2D> ValidMoves;
    for (const FVector2D& MovePos : Moves)
    {
        if (GM->GField->IsValidPosition(MovePos))
        {
            ATile* Tile = GM->GField->GetTileMap()[MovePos];
            if (Tile && Tile->GetTileStatus() == ETileStatus::EMPTY)
            {
                ValidMoves.Add(MovePos);
            }
        }
    }
    bool bCanMove = (ValidMoves.Num() > 0);

    // Calcola le possibili mosse di attacco
    TArray<FVector2D> Attacks = Unit->CalculateAttackMoves();
    TArray<FVector2D> ValidAttacks;
    for (const FVector2D& AttackPos : Attacks)
    {
        if (GM->GField->IsValidPosition(AttackPos))
        {
            ATile* Tile = GM->GField->GetTileMap()[AttackPos];
            // Supponiamo che le unit� nemiche abbiano PlayerOwner == 0
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

    // Se entrambe le azioni sono possibili, esegui "Muovi e poi Attacca"
    if (bCanMove && bCanAttack)
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unit� ID %d -> Muovi e poi Attacca"), Unit->GetGameUnitID());
        FVector2D MovePos = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];
        GM->GField->MoveUnit(Unit, MovePos);

        // Dopo il movimento, ricalcola le possibili mosse di attacco
        Attacks = Unit->CalculateAttackMoves();
        ValidAttacks.Empty();
        for (const FVector2D& AttackPos : Attacks)
        {
            if (GM->GField->IsValidPosition(AttackPos))
            {
                ATile* Tile = GM->GField->GetTileMap()[AttackPos];
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
        if (ValidAttacks.Num() > 0)
        {
            FVector2D AttackPos = ValidAttacks[FMath::RandRange(0, ValidAttacks.Num() - 1)];
            GM->GField->AttackUnit(Unit, AttackPos);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("AI: Unit� ID %d -> Nessun bersaglio disponibile dopo il movimento"), Unit->GetGameUnitID());
        }
    }
    // Se solo attaccare � possibile, esegui "Solo Attacca"
    else if (bCanAttack)
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unit� ID %d -> Solo Attacca"), Unit->GetGameUnitID());
        FVector2D AttackPos = ValidAttacks[FMath::RandRange(0, ValidAttacks.Num() - 1)];
        GM->GField->AttackUnit(Unit, AttackPos);
    }
    // Altrimenti, se solo muovere � possibile, esegui "Solo Muovi"
    else if (bCanMove)
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unit� ID %d -> Solo Muovi"), Unit->GetGameUnitID());
        FVector2D MovePos = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];
        GM->GField->MoveUnit(Unit, MovePos);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AI: Unit� ID %d non pu� n� muoversi n� attaccare"), Unit->GetGameUnitID());
    }
}





