// RandomPlayer.cpp

#include "RandomPlayer.h"
#include "AWGameMode.h"
#include "GameField.h"
#include "GameUnit.h"
#include "EngineUtils.h"

ARandomPlayer::ARandomPlayer()
{
    // Se vuoi differenziare PlayerNumber da 0 (umano)
    PlayerNumber = 1;
}


void ARandomPlayer::OnTurn()
{
    UE_LOG(LogTemp, Warning, TEXT("ARandomPlayer::OnTurn() - Inizio turno AI"));

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        UE_LOG(LogTemp, Error, TEXT("AI OnTurn: GameMode o GameField non trovato"));
        return;
    }

    // Filtra le unità AI (PlayerOwner == 1)
    TArray<AGameUnit*> AIUnits = TrovaLeMieUnita(GM);

    // Se non ho unità, passo subito il turno
    if (AIUnits.Num() == 0)
    {
        GM->NextTurn();
        return;
    }

    // Esegui le mosse AI (in un colpo solo o una per una). 
    // Ad esempio:
    for (AGameUnit* Unit : AIUnits)
    {
        if (!Unit->bHasActed)
        {
            // Fai random move + attacco
            PerformRandomActionOnUnit(Unit);
        }
    }

    // A fine di TUTTE le azioni, passo il turno
    GM->NextTurn();
}


void ARandomPlayer::PerformRandomActionOnUnit(AGameUnit* Unit)
{
    if (!Unit)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformRandomActionOnUnit: unità nulla."));
        return;
    }

    // Se l'unità ha già agito, non eseguire nessuna azione
    if (Unit->bHasActed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unità ID %d ha già agito."), Unit->GetGameUnitID());
        return;
    }

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformRandomActionOnUnit: GameMode o GameField non trovati."));
        return;
    }

    // 1. Calcola le mosse legali per il movimento
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

    // 2. Calcola le mosse di attacco
    TArray<FVector2D> Attacks = Unit->CalculateAttackMoves();
    TArray<FVector2D> ValidAttacks;
    for (const FVector2D& AttackPos : Attacks)
    {
        if (GM->GField->IsValidPosition(AttackPos))
        {
            ATile* Tile = GM->GField->GetTileMap()[AttackPos];
            // Se c'è un'unità nemica (assumendo che le unità nemiche abbiano PlayerOwner == 0)
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

    // Se l'unità non può né muoversi né attaccare, esci
    if (!bCanMove && !bCanAttack)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unità ID %d non può né muoversi né attaccare."), Unit->GetGameUnitID());
        return;
    }

    // 3. Prepara le opzioni
    TArray<int32> Options; // 0 = Muovi+Attacca, 1 = Solo Attacca, 2 = Solo Muovi
    if (bCanMove && bCanAttack)
        Options.Add(0);
    if (bCanAttack)
        Options.Add(1);
    if (bCanMove)
        Options.Add(2);

    int32 ActionChoice = Options[FMath::RandRange(0, Options.Num() - 1)];
    switch (ActionChoice)
    {
    case 0:
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unità ID %d -> Muovi e poi Attacca"), Unit->GetGameUnitID());
        FVector2D MovePos = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];
        GM->GField->MoveUnit(Unit, MovePos);

        // (Opzionale) Aspetta o aggiorna la posizione, poi ricalcola le mosse di attacco
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
    }
    break;
    case 1:
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unità ID %d -> Solo Attacca"), Unit->GetGameUnitID());
        FVector2D AttackPos = ValidAttacks[FMath::RandRange(0, ValidAttacks.Num() - 1)];
        GM->GField->AttackUnit(Unit, AttackPos);
    }
    break;
    case 2:
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unità ID %d -> Solo Muovi"), Unit->GetGameUnitID());
        FVector2D MovePos = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];
        GM->GField->MoveUnit(Unit, MovePos);
    }
    break;
    }

    // Segna che questa unità ha già agito per questo turno
    Unit->bHasActed = true;

    UE_LOG(LogTemp, Log, TEXT("ARandomPlayer::OnTurn() - Fine turno AI"));
    GM->NextTurn();
}
