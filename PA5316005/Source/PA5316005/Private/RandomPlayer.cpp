#include "RandomPlayer.h"
#include "AWGameMode.h"
#include "GameField.h"
#include "GameUnit.h"
#include "AWGameInstance.h"
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
        UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GetWorld() � nullptr!"));
        return;
    }

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM)
    {
        UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GameMode nullo."));
        return;
    }

    if (!GM->GField)
    {
        UE_LOG(LogTemp, Error, TEXT("ARandomPlayer::OnTurn() - GField nullo."));
        return;
    }



    // Filtra le unit� AI che non hanno ancora completato entrambe le azioni
    TArray<AGameUnit*> AIUnits;
    for (auto& Pair : GM->GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;
        if (Unit && Unit->GetPlayerOwner() == 1 && !(Unit->bHasMoved && Unit->bHasAttacked))
        {
            AIUnits.Add(Unit);
        }
    }

    if (AIUnits.Num() == 0)
    {
        // Se tutte le unit� hanno completato le azioni, passa il turno
        GM->NextTurn();
        return;
    }

    // Inizializza la sequenza per gestire le unit� una alla volta
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

    // Se abbiamo finito di processare le unit�, passa il turno
    while (SequenceIndex < UnitsSequence.Num() && !IsValid(UnitsSequence[SequenceIndex]))
    {
        SequenceIndex++;
    }
    if (SequenceIndex >= UnitsSequence.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Tutte le unit� hanno completato le azioni. Passo il turno."));
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

    // Mostra le tile di movimento (e attacco) per l'unit� corrente
    UE_LOG(LogTemp, Warning, TEXT("AI -> Mostro le tile per l'unit� ID=%d"), CurrentUnit->GetGameUnitID());
    GM->GField->ShowLegalMovesForUnit(CurrentUnit);

    // Aspetta un po' per dare tempo di visualizzare le tile (ad esempio 2 secondi)
    FTimerHandle ShowTilesDelay;
    GetWorldTimerManager().SetTimer(ShowTilesDelay, [this, CurrentUnit, GM]()
        {
            // Esegui l'azione casuale (muovi e/o attacca)
            PerformRandomActionOnUnit(CurrentUnit);
            CurrentUnit->bHasMoved = true;
            CurrentUnit->bHasAttacked = true;

            // Dopo che l'unit� ha completato la sua azione, aspetta un ulteriore ritardo (ad esempio 1 secondo)
            FTimerHandle NextUnitDelay;
            GetWorldTimerManager().SetTimer(NextUnitDelay, [this, GM]()
                {
                    // Resetta le evidenziazioni (tile) e passa all'unit� successiva
                    GM->GField->ResetGameStatusField();
                    SequenceIndex++;
                    DoNextUnitAction();
                }, 1.0f, false); // Ritardo aggiuntivo tra le azioni delle unit�

        }, 2.0f, false); // Ritardo iniziale per visualizzare le tile
}



void ARandomPlayer::PerformRandomActionOnUnit(AGameUnit* Unit)
{
    if (!Unit) return;

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField) return;

    // ---------------------------------------------------------
    // 1) Calcolo le possibili mosse di movimento
    // ---------------------------------------------------------
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

    // ---------------------------------------------------------
    // 2) Calcolo le possibili mosse di attacco
    // ---------------------------------------------------------
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
                // Nemico = PlayerOwner == 0
                if (Target && Target->GetPlayerOwner() == 0)
                {
                    ValidAttacks.Add(AttackPos);
                }
            }
        }
    }
    bool bCanAttack = (ValidAttacks.Num() > 0);

    // ---------------------------------------------------------
    // 3) Decido l'azione: "Muovi e poi Attacca" / "Solo Muovi" / "Solo Attacca" / Nessuna
    // ---------------------------------------------------------
    if (bCanMove && bCanAttack)
    {
        UE_LOG(LogTemp, Log, TEXT("AI -> Unit� ID %d: Muovi e poi Attacca"), Unit->GetGameUnitID());
        FVector2D MoveChoice = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];

        // Muovo con callback
        GM->GField->MoveUnit(Unit, MoveChoice, [this, Unit, GM]()
            {
                // Subito dopo il movimento, resetto le tile di movimento
                GM->GField->ResetGameStatusField();

                // Mostro le tile di attacco
                GM->GField->ShowLegalAttackOptionsForUnit(Unit);

                // Attendo un attimo per "visualizzare" le tile rosse
                FTimerHandle AttackDelay;
                GetWorld()->GetTimerManager().SetTimer(
                    AttackDelay,
                    [this, Unit, GM]()
                    {
                        // Ricalcolo i bersagli validi
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

                        // Reset finale delle tile rosse
                        GM->GField->ResetGameStatusField();
                    },
                    0.5f, // 0.5 secondi
                    false
                );
            });
    }
    else if (bCanAttack)
    {
        UE_LOG(LogTemp, Log, TEXT("AI -> Unit� ID %d: Solo Attacca"), Unit->GetGameUnitID());
        FVector2D AttackChoice = ValidAttacks[FMath::RandRange(0, ValidAttacks.Num() - 1)];
        GM->GField->AttackUnit(Unit, AttackChoice);

        // Dopo l'attacco, resetto
        GM->GField->ResetGameStatusField();
    }
    else if (bCanMove)
    {
        UE_LOG(LogTemp, Log, TEXT("AI -> Unit� ID %d: Solo Muovi"), Unit->GetGameUnitID());
        FVector2D MoveChoice = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];

        // Muovo e poi resetto
        GM->GField->MoveUnit(Unit, MoveChoice, [GM]()
            {
                GM->GField->ResetGameStatusField();
            });
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AI -> Unit� ID %d non pu� n� muoversi n� attaccare."), Unit->GetGameUnitID());
    }
}







