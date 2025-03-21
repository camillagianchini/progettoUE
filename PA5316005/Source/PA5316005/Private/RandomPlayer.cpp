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
        UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GetWorld() è nullptr!"));
        return;
    }

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM)
    {
        UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GetAuthGameMode() è nullptr!"));
        return;
    }

    if (!GM->GField)
    {
        UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GField è nullptr!"));
        return;
    }

    // Filtra le unità AI che non hanno ancora completato entrambe le azioni
    TArray<AGameUnit*> AIUnits;
    for (auto& Pair : GM->GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;
        // Includi l’unità se almeno una delle due azioni non è stata effettuata
        if (Unit && Unit->GetPlayerOwner() == 1 && !(Unit->bHasMoved && Unit->bHasAttacked))
        {
            AIUnits.Add(Unit);
        }
    }

    if (AIUnits.Num() == 0)
    {
        // Se tutte le unità hanno completato, passa il turno
        GM->NextTurn();
        return;
    }

    // Inizializza la sequenza per gestire le unità una alla volta
    SequenceIndex = 0;
    UnitsSequence = AIUnits;
    DoNextUnitAction();
}

void ARandomPlayer::DoNextUnitAction()
{
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM) return;

    // Se abbiamo finito di processare le unità, passa il turno
    if (SequenceIndex >= UnitsSequence.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Tutte le unità hanno completato le azioni. Passo il turno."));
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

    // Mostra le opzioni legali (movimento e attacco) per l'unità corrente
    UE_LOG(LogTemp, Warning, TEXT("Chiamato ShowLegalMovesForUnit per unità ID=%d"), CurrentUnit->GetGameUnitID());
    if (GM->GField)
    {
        GM->GField->ShowLegalMovesForUnit(CurrentUnit);
    }

    // Attendi 2 secondi per "visualizzare" le opzioni e poi esegui l'azione
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(
        TimerHandle,
        [this, CurrentUnit]()
        {
            // Esegui l'azione per la unità corrente
            PerformRandomActionOnUnit(CurrentUnit);

            // Segna entrambe le azioni come completate
            CurrentUnit->bHasMoved = true;
            CurrentUnit->bHasAttacked = true;

            // Passa alla prossima unità
            SequenceIndex++;
            DoNextUnitAction();
        },
        2.0f,
        false
    );
}

void ARandomPlayer::PerformRandomActionOnUnit(AGameUnit* Unit)
{
    if (!Unit)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformRandomActionOnUnit: unità nulla."));
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
            if (Tile && Tile->GetTileStatus() == ETileStatus::OCCUPIED)
            {
                AGameUnit* Target = Tile->GetGameUnit();
                // Considera come bersaglio una unità nemica (playerOwner == 0)
                if (Target && Target->GetPlayerOwner() == 0)
                {
                    ValidAttacks.Add(AttackPos);
                }
            }
        }
    }
    bool bCanAttack = (ValidAttacks.Num() > 0);

    // Se entrambe le azioni sono possibili, "Muovi e poi Attacca"
    if (bCanMove && bCanAttack)
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unità ID %d -> Muovi e poi Attacca"), Unit->GetGameUnitID());
        FVector2D ChosenMove = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];

        // Muovo l’unità con callback
        GM->GField->MoveUnit(Unit, ChosenMove, [GM, Unit, this]()
            {
                // (1) Dopo il movimento, resetto le tile verdi
                GM->GField->ResetGameStatusField();

                // (2) Mostro le tile d'attacco (rosse) giusto per feedback visivo
                GM->GField->ShowLegalAttackOptionsForUnit(Unit);

                // (3) Attendo un brevissimo delay prima di attaccare
                FTimerHandle AttackTimer;
                GetWorld()->GetTimerManager().SetTimer(
                    AttackTimer,
                    [GM, Unit]()
                    {
                        // Ricalcola possibili attacchi
                        TArray<FVector2D> NewAttacks = Unit->CalculateAttackMoves();
                        TArray<FVector2D> NewValidAttacks;
                        for (const FVector2D& AttackPos : NewAttacks)
                        {
                            if (GM->GField->IsValidPosition(AttackPos))
                            {
                                ATile* Tile = GM->GField->GetTileMap()[AttackPos];
                                if (Tile && Tile->GetTileStatus() == ETileStatus::OCCUPIED)
                                {
                                    AGameUnit* Target = Tile->GetGameUnit();
                                    if (Target && Target->GetPlayerOwner() == 0)
                                    {
                                        NewValidAttacks.Add(AttackPos);
                                    }
                                }
                            }
                        }
                        if (NewValidAttacks.Num() > 0)
                        {
                            FVector2D ChosenAttack = NewValidAttacks[FMath::RandRange(0, NewValidAttacks.Num() - 1)];
                            GM->GField->AttackUnit(Unit, ChosenAttack);
                        }

                        // (4) A prescindere dall’esito, resetto le tile dopo l’attacco
                        GM->GField->ResetGameStatusField();
                    },
                    0.5f, // delay di 0.5 secondi
                    false
                );
            });
    }
    // Se solo attaccare è possibile
    else if (bCanAttack)
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unità ID %d -> Solo Attacca"), Unit->GetGameUnitID());
        FVector2D ChosenAttack = ValidAttacks[FMath::RandRange(0, ValidAttacks.Num() - 1)];

        GM->GField->AttackUnit(Unit, ChosenAttack);

        // Subito dopo l’attacco, resetto le tile rosse
        GM->GField->ResetGameStatusField();
    }
    // Se solo muovere è possibile
    else if (bCanMove)
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unità ID %d -> Solo Muovi"), Unit->GetGameUnitID());
        FVector2D ChosenMove = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];

        // Muovo l'unità e poi resetto
        GM->GField->MoveUnit(Unit, ChosenMove, [GM]()
            {
                GM->GField->ResetGameStatusField();
            });
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AI: Unità ID %d non può né muoversi né attaccare"), Unit->GetGameUnitID());
    }
}







