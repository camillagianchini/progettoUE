// RandomPlayer.cpp

#include "RandomPlayer.h"
#include "AWGameMode.h"
#include "GameField.h"
#include "GameUnit.h"
#include "EngineUtils.h"

void ARandomPlayer::OnTurn()
{
    UE_LOG(LogTemp, Log, TEXT("ARandomPlayer::OnTurn() - Inizio turno AI"));

    // Ottieni il riferimento al GameMode
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        UE_LOG(LogTemp, Error, TEXT("AI OnTurn: GameMode o GameField non trovato"));
        return;
    }

    // Filtra le unità AI (PlayerOwner == 1)
    TArray<AGameUnit*> AIUnits;
    for (auto& Pair : GM->GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;
        if (Unit && Unit->GetPlayerOwner() == 1)
        {
            AIUnits.Add(Unit);
        }
    }
    if (AIUnits.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Nessuna unità AI disponibile."));
        // Nessuna unità AI? Termina subito
        GM->NextTurn();
        return;
    }

    // Esegui un'azione per ciascuna unità dell'AI (es. 2 unità: Sniper e Brawler)
    for (AGameUnit* Unit : AIUnits)
    {
        // Esegui la funzione che sceglie random se muovere e attaccare, ecc.
        PerformRandomActionOnUnit(Unit);
    }

    // Al termine delle azioni di tutte le unità, passa il turno
    UE_LOG(LogTemp, Log, TEXT("ARandomPlayer::OnTurn() - Fine turno AI"));
    GM->NextTurn();
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

    // 1. Calcola le mosse legali di movimento
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
    bool bCanMove = ValidMoves.Num() > 0;

    // 2. Calcola le mosse di attacco
    TArray<FVector2D> Attacks = Unit->CalculateAttackMoves();
    TArray<FVector2D> ValidAttacks;
    for (const FVector2D& AttackPos : Attacks)
    {
        if (GM->GField->IsValidPosition(AttackPos))
        {
            ATile* Tile = GM->GField->GetTileMap()[AttackPos];
            // Se c'è un'unità nemica (PlayerOwner == 0)
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
    bool bCanAttack = ValidAttacks.Num() > 0;

    // Se non può fare nulla
    if (!bCanMove && !bCanAttack)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unità ID %d non può né muoversi né attaccare."), Unit->GetGameUnitID());
        return;
    }

    // 3. Prepara un array di opzioni
    TArray<int32> Options; // 0 = Muovi+Attacca, 1 = SoloAttacca, 2 = SoloMuovi
    if (bCanMove && bCanAttack)
    {
        Options.Add(0); // Muovi e poi attacca
    }
    if (bCanAttack)
    {
        Options.Add(1); // Solo attacca
    }
    if (bCanMove)
    {
        Options.Add(2); // Solo muovi
    }

    // Scegli una delle opzioni
    int32 ActionChoice = Options[FMath::RandRange(0, Options.Num() - 1)];
    switch (ActionChoice)
    {
    case 0: // Muovi e poi attacca
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unità ID %d -> Muovi e poi Attacca"), Unit->GetGameUnitID());
        // Scegli un move
        FVector2D MovePos = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];
        GM->GField->MoveUnit(Unit, MovePos);

        // Ricalcola le mosse di attacco dopo il movimento
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
    case 1: // Solo Attacca
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unità ID %d -> Solo Attacca"), Unit->GetGameUnitID());
        FVector2D AttackPos = ValidAttacks[FMath::RandRange(0, ValidAttacks.Num() - 1)];
        GM->GField->AttackUnit(Unit, AttackPos);
    }
    break;
    case 2: // Solo Muovi
    {
        UE_LOG(LogTemp, Log, TEXT("AI: Unità ID %d -> Solo Muovi"), Unit->GetGameUnitID());
        FVector2D MovePos = ValidMoves[FMath::RandRange(0, ValidMoves.Num() - 1)];
        GM->GField->MoveUnit(Unit, MovePos);
    }
    break;
    }
}



