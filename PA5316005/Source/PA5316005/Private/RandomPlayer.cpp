// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomPlayer.h"

// Sets default values
ARandomPlayer::ARandomPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GameInstance = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

}

// Called when the game starts or when spawned
void ARandomPlayer::BeginPlay()
{
	Super::BeginPlay();
	GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
	
}

// Called every frame
void ARandomPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARandomPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ARandomPlayer::OnTurn()
{
    GameInstance->SetTurnMessage(TEXT("AI Turn"));

    // Esempio semplice: se sono disponibili attacchi, con probabilità 50% attacca, altrimenti muove.
    if (FMath::RandBool())
    {
        MakeRandomAttack();
    }
    else
    {
        MakeRandomMove();
    }
}


void ARandomPlayer::OnWin()
{
	GameInstance->SetTurnMessage(TEXT("AI Wins!!"));
	GameInstance->IncrementScoreAiPlayer();
}

void ARandomPlayer::OnLose()
{
    GameInstance->SetTurnMessage(TEXT("AI Loses!!"));
}

// Esegue una mossa casuale (solo spostamento)
void ARandomPlayer::MakeRandomMove()
{
    if (!GameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: GameMode non valido per MakeRandomMove."));
        return;
    }

    // Recupera le unità dell'AI
    TArray<AGameUnit*> MyUnits;
    for (const auto& Pair : GameMode->GameUnitMap)
    {
        if (Pair.Value && Pair.Value->GetPlayerOwner() == PlayerNumber)
        {
            MyUnits.Add(Pair.Value);
        }
    }

    if (MyUnits.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessuna unità disponibile per il movimento."));
        GameMode->TurnNextPlayer();
        return;
    }

    // Seleziona una unità casuale
    int32 RandIndex = FMath::RandRange(0, MyUnits.Num() - 1);
    AGameUnit* ChosenUnit = MyUnits[RandIndex];
    UE_LOG(LogTemp, Log, TEXT("RandomPlayer (Move): Unità scelta (ID: %d)"), ChosenUnit->GetGameUnitID());

    // Calcola le mosse legali di movimento per l'unità
    TArray<FVector2D> MoveOptions = ChosenUnit->CalculateLegalMoves();
    if (MoveOptions.Num() > 0)
    {
        int32 MoveIndex = FMath::RandRange(0, MoveOptions.Num() - 1);
        FVector2D Destination = MoveOptions[MoveIndex];
        UE_LOG(LogTemp, Log, TEXT("RandomPlayer (Move): Unità %d si muove verso %s."), ChosenUnit->GetGameUnitID(), *Destination.ToString());

        GameMode->DoMove(Destination, true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomPlayer (Move): Nessuna mossa legale per l'unità %d."), ChosenUnit->GetGameUnitID());
        GameMode->TurnNextPlayer();
    }
}

// Esegue un attacco casuale senza movimento
void ARandomPlayer::MakeRandomAttack()
{
    if (!GameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: GameMode non valido per MakeRandomAttack."));
        return;
    }

    // Recupera le unità dell'AI
    TArray<AGameUnit*> MyUnits;
    for (const auto& Pair : GameMode->GameUnitMap)
    {
        if (Pair.Value && Pair.Value->GetPlayerOwner() == PlayerNumber)
        {
            MyUnits.Add(Pair.Value);
        }
    }

    if (MyUnits.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessuna unità disponibile per l'attacco."));
        GameMode->TurnNextPlayer();
        return;
    }

    // Seleziona una unità casuale
    int32 RandIndex = FMath::RandRange(0, MyUnits.Num() - 1);
    AGameUnit* ChosenUnit = MyUnits[RandIndex];
    UE_LOG(LogTemp, Log, TEXT("RandomPlayer (Attack): Unità scelta (ID: %d)"), ChosenUnit->GetGameUnitID());

    // Calcola le mosse d'attacco per l'unità (senza movimento)
    // Supponiamo che il metodo CalculateAttackMoves() restituisca un array di destinazioni d'attacco
    TArray<FVector2D> AttackOptions = ChosenUnit->CalculateAttackMoves();
    if (AttackOptions.Num() > 0)
    {
        int32 AttackIndex = FMath::RandRange(0, AttackOptions.Num() - 1);
        FVector2D AttackDestination = AttackOptions[AttackIndex];
        UE_LOG(LogTemp, Log, TEXT("RandomPlayer (Attack): Unità %d attacca direttamente verso %s."), ChosenUnit->GetGameUnitID(), *AttackDestination.ToString());

        GameMode->DoAttack(AttackDestination, true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomPlayer (Attack): Nessun attacco legale per l'unità %d."), ChosenUnit->GetGameUnitID());
        GameMode->TurnNextPlayer();
    }
}

void ARandomPlayer::PlaceUnitsRandomly()
{
    if (!GameMode || !GameMode->GField)
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: GameMode o GField non validi."));
        return;
    }

    // Prepara un array con le posizioni vuote nel GameField
    TArray<FVector2D> EmptyTiles;
    for (const TPair<FVector2D, ATile*>& Pair : GameMode->GField->TileMap)
    {
        ATile* Tile = Pair.Value;
        if (Tile && Tile->GetTileStatus() == ETileStatus::EMPTY)
        {
            EmptyTiles.Add(Pair.Key);
        }
    }

    if (EmptyTiles.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessuna tile vuota disponibile."));
        return;
    }

    // Posiziona lo Sniper se non ancora piazzato per questo giocatore
    if (!GameMode->bSniperPlaced.Contains(PlayerNumber) || !GameMode->bSniperPlaced[PlayerNumber])
    {
        int32 RandIndex = FMath::RandRange(0, EmptyTiles.Num() - 1);
        FVector2D ChosenPos = EmptyTiles[RandIndex];
        GameMode->PlaceUnit(PlayerNumber, ChosenPos, EGameUnitType::SNIPER);
        UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Posizionato SNIPER in %s."), *ChosenPos.ToString());
    }

    // Ricostruisci l'array delle tile vuote (poiché una potrebbe essere ora occupata)
    EmptyTiles.Empty();
    for (const TPair<FVector2D, ATile*>& Pair : GameMode->GField->TileMap)
    {
        ATile* Tile = Pair.Value;
        if (Tile && Tile->GetTileStatus() == ETileStatus::EMPTY)
        {
            EmptyTiles.Add(Pair.Key);
        }
    }

    if (EmptyTiles.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessuna tile vuota disponibile per posizionare il Brawler."));
        return;
    }

    // Posiziona il Brawler se non ancora piazzato
    if (!GameMode->bBrawlerPlaced.Contains(PlayerNumber) || !GameMode->bBrawlerPlaced[PlayerNumber])
    {
        int32 RandIndex = FMath::RandRange(0, EmptyTiles.Num() - 1);
        FVector2D ChosenPos = EmptyTiles[RandIndex];
        GameMode->PlaceUnit(PlayerNumber, ChosenPos, EGameUnitType::BRAWLER);
        UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Posizionato BRAWLER in %s."), *ChosenPos.ToString());
    }
}