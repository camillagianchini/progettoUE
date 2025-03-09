// Fill out your copyright notice in the Description page of Project Settings.


#include "AWGameMode.h"

#include "AWGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AAWGameMode::AAWGameMode()
{
    // Impostazione di default
    CurrentPhase = EGamePhase::PreGame;

    // Abbiamo 2 unità per giocatore da posizionare
    HumanUnitsToPlace = 2;
    AIUnitsToPlace = 2;

    bHumanStarts = false;
}

void AAWGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Avvio della fase di PreGame
    StartPreGame();
}

void AAWGameMode::StartPreGame()
{
    CurrentPhase = EGamePhase::PreGame;

    // Trova gli attori di tipo APawn che implementano IPlayerInterface
    for (TActorIterator<APawn> PawnItr(GetWorld()); PawnItr; ++PawnItr)
    {
        APawn* FoundPawn = *PawnItr;
        if (FoundPawn->Implements<UPlayerInterface>())
        {
            IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(FoundPawn);
            if (PlayerInterface)
            {
                if (PlayerInterface->PlayerNumber == 1)
                {
                    HumanPlayer = FoundPawn;
                    // Possediamo il pawn umano con il PlayerController 0
                    APlayerController* PC = GetWorld()->GetFirstPlayerController();
                    if (PC)
                    {
                        PC->Possess(Cast<APawn>(HumanPlayer.GetObject()));
                    }
                }
                else if (PlayerInterface->PlayerNumber == 2)
                {
                    AIPlayer = FoundPawn;
                }
            }
        }
    }

    // Controllo se abbiamo assegnato entrambi i giocatori
    if (!HumanPlayer || !AIPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("Errore: Non sono stati trovati entrambi i giocatori!"));
        return;
    }

    // Lancio della moneta
    DoCoinToss();

    // Inizia la fase di posizionamento
    StartPlacingUnits();
}



void AAWGameMode::DoCoinToss()
{
    // Esempio: 50% di probabilità
    bHumanStarts = (FMath::RandRange(0, 1) == 0);

    if (bHumanStarts)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Human Player ha vinto il lancio di moneta!"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("AI Player ha vinto il lancio di moneta!"));
    }
}

void AAWGameMode::StartPlacingUnits()
{
    // Il giocatore che ha vinto il coin toss piazza per primo.
    // Potresti richiamare una funzione su HumanPlayer / AIPlayer
    // che spawna la sua prima unità in una posizione scelta.
    // 
    // Al termine del posizionamento, chiamerai OnUnitPlaced(true) o OnUnitPlaced(false)
    // a seconda di chi ha finito di piazzare.
    // 
    // N.B. L'implementazione reale dipenderà da come gestisci input e scelte AI.
}

void AAWGameMode::OnUnitPlaced(bool bIsHuman)
{
    if (bIsHuman)
    {
        HumanUnitsToPlace--;
    }
    else
    {
        AIUnitsToPlace--;
    }

    // Se un player ha ancora unità da piazzare, tocca a lui.
    // Se il player ha finito, tocca all'altro.
    // 
    // ESEMPIO: 
    if (HumanUnitsToPlace > 0 || AIUnitsToPlace > 0)
    {
        // Alterna al prossimo player in base a chi ha appena piazzato
        // In un codice più rifinito, potresti mantenere CurrentPlayer = (bIsHuman ? AI : Human) e chiamare l'azione di posizionamento
    }
    else
    {
        // Tutte le unità sono state piazzate -> avvia la partita
        StartGame();
    }
}

void AAWGameMode::StartGame()
{
    CurrentPhase = EGamePhase::InGame;

    if (bHumanStarts)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Inizia il turno dell'umano"));
        IPlayerInterface::Execute_OnTurn(HumanPlayer.GetObject());
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Inizia il turno dell'AI"));
        IPlayerInterface::Execute_OnTurn(AIPlayer.GetObject());
    }
}


void AAWGameMode::EndMatch(bool bHumanWon)
{
    CurrentPhase = EGamePhase::EndGame;

    if (bHumanWon)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("UMANO VINCE!"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("AI VINCE!"));
    }

    // Qui potresti fare un level transition, tornare a un menu, o bloccare l'input
    // ...
}
