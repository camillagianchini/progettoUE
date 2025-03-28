#include "AWGameMode.h"
#include "GameField.h"
#include "Tile.h"
#include "AWPlayerController.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "EngineUtils.h"
#include "CoinTossWidget.h"
#include "MovesPanel.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AAWGameMode::AAWGameMode()
{
    // Imposta il PlayerController e il Pawn di default
    PlayerControllerClass = AAWPlayerController::StaticClass();
    DefaultPawnClass = AHumanPlayer::StaticClass();

    // Inizializza lo stato di gioco
    CurrentPlayer = 0;      // Di default il giocatore 0 (Human) inizia
    FieldSize = 25;         // Griglia 25x25
    bFirstBattleTurn = false;

    // Inizializza le mappe per il posizionamento delle unit� per i due giocatori (0 e 1)
    bSniperPlaced.Add(0, false);
    bSniperPlaced.Add(1, false);
    bBrawlerPlaced.Add(0, false);
    bBrawlerPlaced.Add(1, false);

    // Carica il blueprint del GameField
    static ConstructorHelpers::FClassFinder<AGameField> GameFieldBPClass(TEXT("/Game/Blueprints/BP_GameField"));
    if (GameFieldBPClass.Succeeded())
    {
        GameFieldClass = GameFieldBPClass.Class;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BP_GameField non trovato!"));
    }


    // Stato iniziale del gioco
    bIsGameOver = false;
    MoveCounter = 0;
    CurrentPhase = EGamePhase::Placement;
    bIsAITurnInProgress = false;

 

}

void AAWGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    if (CoinTossWidgetClass)
    {
        CoinTossWidget = CreateWidget<UCoinTossWidget>(GetWorld(), CoinTossWidgetClass);
        if (CoinTossWidget)
        {
            // Ti sottoscrivi all�evento OnCoinTossCompleted
            CoinTossWidget->OnCoinTossCompleted.AddDynamic(this, &AAWGameMode::CoinTossForStartingPlayer);

            CoinTossWidget->AddToViewport();

            FInputModeGameAndUI InputMode;
            // Mettiamo il focus sul widget stesso
            InputMode.SetWidgetToFocus(CoinTossWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            GetWorld()->GetFirstPlayerController()->SetInputMode(InputMode);

            // Mostriamo il cursore
            GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;
            // Imposti bCoinTossActive a true
            bCoinTossActive = true;

        }
    }
    // In AAWGameMode::BeginPlay()
    if (MovesPanelClass)
    {
        MovesPanel = CreateWidget<UMovesPanel>(GetWorld(), MovesPanelClass);
        if (MovesPanel)
        {
            MovesPanel->AddToViewport();
        }
    }


    if (UnitListWidgetClass)
    {
        UnitListWidget = CreateWidget<UUserWidget>(GetWorld(), UnitListWidgetClass);
        if (UnitListWidget)
        {
            UnitListWidget->AddToViewport();
        }
    }


    // Ottieni il riferimento al HumanPlayer (Pawn del primo PlayerController)
    AHumanPlayer* HumanPlayer = GetWorld()->GetFirstPlayerController()->GetPawn<AHumanPlayer>();
    if (!IsValid(HumanPlayer))
    {
        UE_LOG(LogTemp, Error, TEXT("No player of type '%s' was found."), *AHumanPlayer::StaticClass()->GetName());
        return;
    }


    // Aggiungi i giocatori: HumanPlayer (indice 0) e AI (indice 1)
    Players.Add(HumanPlayer);
    PlayerNames.Add(0, "Human");


    auto* IA = GetWorld()->SpawnActor<ARandomPlayer>(FVector::ZeroVector, FRotator::ZeroRotator);
    Players.Add(IA);
    PlayerNames.Add(1, "IA");


    // Spawn del GameField
    if (GameFieldClass != nullptr)
    {
        GField = GetWorld()->SpawnActor<AGameField>(GameFieldClass);
        GField->Size = FieldSize;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Game Field is null"));
    }

    // Calcola la posizione della camera e posiziona il HumanPlayer
    float CameraPosX = ((GField->TileSize * FieldSize) + ((FieldSize - 1) * GField->TileSize * GField->CellPadding)) * 0.5f;
    float Zposition = 3500.0f; // Regola in base alla tua scena
    FVector CameraPos(CameraPosX, CameraPosX, Zposition);
    FRotator CameraRot(-90.0f, 0.0f, 0.0f);
    HumanPlayer->SetActorLocationAndRotation(CameraPos, CameraRot);


}

// Aggiungi un parametro per ricevere il risultato
void AAWGameMode::CoinTossForStartingPlayer(int32 CoinResult)
{
    bCoinTossActive = false;

    // Se CoinResult == 0 => Human inizia, altrimenti AI
    StartingPlayer = CoinResult;
    CurrentPlayer = CoinResult;

    UE_LOG(LogTemp, Warning, TEXT("Coin toss concluso, esito: %d"), CoinResult);

    // Se il widget � valido, lo rimango visibile per 2 secondi prima di rimuoverlo
    if (CoinTossWidget)
    {
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [this]()
            {
                if (CoinTossWidget)
                {
                    CoinTossWidget->RemoveFromParent();
                    CoinTossWidget = nullptr;
                    FInputModeGameOnly GameOnlyMode;
                    GetWorld()->GetFirstPlayerController()->SetInputMode(GameOnlyMode);
                    // Se vuoi ancora il cursore visibile in gioco, rimetti bShowMouseCursor = true;
                    // Oppure false, se preferisci che sparisca
                    GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;
                }
                // Dopo la rimozione, procedo col posizionamento delle unit�
                PlaceUnitForCurrentPlayer();
            }, 2.0f, false);
    }
    else
    {
        // Se il widget non � valido, procedo subito
        PlaceUnitForCurrentPlayer();
    }
}







void AAWGameMode::PlaceUnitForCurrentPlayer()
{
    if (CurrentPlayer == 1) // Caso AI (RandomPlayer)
    {

        // Ottieni una tile libera casuale dal GameField
        ATile* RandomTile = GField->GetRandomFreeTile();
        if (RandomTile)
        {
            FVector2D Position = RandomTile->GetGridPosition();
            bool bPlaceSniper = false;

            // Determina quale unit� posizionare per l'AI
            if (!bSniperPlaced[1])
            {
                bPlaceSniper = true;
            }
            else if (!bBrawlerPlaced[1])
            {
                bPlaceSniper = false;
            }
            else
            {
                bPlaceSniper = FMath::RandBool();
            }

            UWorld* World = GetWorld();
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            FVector SpawnLocation = GField->GetRelativePositionByXYPosition(Position.X, Position.Y);
            // Aggiungiamo un offset Z se necessario, ad esempio:
            SpawnLocation.Z += 5.0f;

            if (bPlaceSniper)
            {
                if (AISniperClass) // Usa il blueprint assegnato
                {
                    ASniper* SpawnedUnit = World->SpawnActor<ASniper>(AISniperClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    if (SpawnedUnit)
                    {
                        bSniperPlaced.Add(1, true);
                        SpawnedUnit->SetPlayerOwner(1);
                        SpawnedUnit->SetGridPosition(Position.X, Position.Y);
                        // Aggiorna la tile: imposta lo stato a OCCUPIED e associa l'unit�


                        UE_LOG(LogTemp, Log, TEXT("AI ha piazzato uno Sniper in %s"), *Position.ToString());
                        if (GField && GField->TileMap.Contains(Position))
                        {
                            ATile* Tile = GField->TileMap[Position];
                            if (Tile)
                            {
                                Tile->SetTileStatus(1, ETileStatus::OCCUPIED, SpawnedUnit);
                            }
                        }
                        int32 NewUnitKey = GField->GameUnitMap.Num();
                        GField->GameUnitMap.Add(NewUnitKey, SpawnedUnit);

                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("AISniperClass non assegnato!"));
                }
            }
            else
            {
                if (AIBrawlerClass) // Usa il blueprint assegnato
                {
                    ABrawler* SpawnedUnit = World->SpawnActor<ABrawler>(AIBrawlerClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    if (SpawnedUnit)
                    {
                        bBrawlerPlaced.Add(1, true);
                        SpawnedUnit->SetPlayerOwner(1);
                        SpawnedUnit->SetGridPosition(Position.X, Position.Y);
                        UE_LOG(LogTemp, Log, TEXT("AI ha piazzato un Brawler in %s"), *Position.ToString());
                        if (GField && GField->TileMap.Contains(Position))
                        {
                            ATile* Tile = GField->TileMap[Position];
                            if (Tile)
                            {
                                Tile->SetTileStatus(1, ETileStatus::OCCUPIED, SpawnedUnit);
                            }
                        }
                        int32 NewUnitKey = GField->GameUnitMap.Num();
                        GField->GameUnitMap.Add(NewUnitKey, SpawnedUnit);
                    }

                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("AIBrawlerClass non assegnato!"));
                }

            }

        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Nessuna tile libera trovata per l'AI!"));
        }
        // Passa il turno al giocatore umano dopo il posizionamento dell'AI
        UE_LOG(LogTemp, Warning, TEXT("piazzare humanplayer!"));
        NextTurn();
    }
    else // Caso Human: il posizionamento umano viene gestito tramite l'input (click) in AHumanPlayer::OnClick()
    {
        // L'input del giocatore umano verr� gestito nel metodo OnClick()
    }

    // Controlla se la fase di posizionamento � terminata
    if (bSniperPlaced[0] && bBrawlerPlaced[0] && bSniperPlaced[1] && bBrawlerPlaced[1])
    {
        // Tutte le unit� sono state posizionate: passa alla fase di battaglia
        CurrentPhase = EGamePhase::Battle;
        UE_LOG(LogTemp, Log, TEXT("Tutte le unit� posizionate. Passaggio alla fase di battaglia."));



        // Ora chiami NextTurn(), e se StartingPlayer == 1, parte l�AI
        NextTurn();
    }


}

int32 AAWGameMode::GetNextPlayer(int32 Player)
{
    // Avanza di 1
    Player++;

    // Se sforiamo l�ultimo indice, torniamo a 0
    if (!Players.IsValidIndex(Player))
    {
        Player = 0;
    }
    return Player;
}


void AAWGameMode::NextTurn()
{
    if (bIsGameOver)
    {
        return;
    }

    // Fase di Placement
    if (CurrentPhase == EGamePhase::Placement)
    {
        bool bHumanDone = bSniperPlaced.FindRef(0) && bBrawlerPlaced.FindRef(0);
        bool bAIDone = bSniperPlaced.FindRef(1) && bBrawlerPlaced.FindRef(1);
        if (!(bHumanDone && bAIDone))
        {
            CurrentPlayer = GetNextPlayer(CurrentPlayer);
            UE_LOG(LogTemp, Log, TEXT("Turno cambiato (placement). CurrentPlayer: %d"), CurrentPlayer);
            SelectedUnit = nullptr;
            PlaceUnitForCurrentPlayer();
            return;
        }


        else
        {
            CurrentPhase = EGamePhase::Battle;
            // Imposta il primo turno di battle senza alternare il giocatore
            CurrentPlayer = StartingPlayer;
            UE_LOG(LogTemp, Log, TEXT("Tutte le unit� posizionate. Passaggio alla fase di battaglia."));
            // Imposta un flag per indicare che � il primo turno di battle
            bFirstBattleTurn = true;
           
        }
    }
    // Fase di Battle
    if (CurrentPhase == EGamePhase::Battle)
    {
        if (bFirstBattleTurn)
        {
            // Il primo turno di battle: non alternare il CurrentPlayer, solo resetta le azioni
            bFirstBattleTurn = false;
        }
        else
        {
            CurrentPlayer = GetNextPlayer(CurrentPlayer);
        }

        // Reset delle azioni per il giocatore corrente
        ResetActionsForPlayer(CurrentPlayer);
        UE_LOG(LogTemp, Log, TEXT("Turno cambiato (battle). CurrentPlayer: %d"), CurrentPlayer);

        if (CurrentPlayer == 1)
        {
            ARandomPlayer* AIPlayer = Cast<ARandomPlayer>(Players[1]);
            if (AIPlayer)
            {
                // Se l'AI � gi� in esecuzione, non farlo partire di nuovo
                if (!bIsAITurnInProgress)
                {
                    bIsAITurnInProgress = true;
                    AIPlayer->OnTurn();
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("RandomPlayer non trovato in Players[1]!"));
            }
        }
        else if (CurrentPlayer == 0)
        {
            bIsAITurnInProgress = false; // L'AI non sta eseguendo
            AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(Players[0]);
            if (HumanPlayer)
            {
                HumanPlayer->OnTurn();
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("HumanPlayer non trovato in Players[0]!"));
            }
        }
    }

}



bool AAWGameMode::AllUnitsHaveActed(int32 Player)
{
    for (auto& Pair : GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;
        // L'unit� ha terminato il turno se ha sia mosso che attaccato
        if (Unit && Unit->GetPlayerOwner() == Player && !(Unit->bHasMoved && Unit->bHasAttacked))
        {
            return false;
        }
    }
    return true;
}

void AAWGameMode::ResetActionsForPlayer(int32 Player)
{
    for (auto& Pair : GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;
        if (Unit && Unit->GetPlayerOwner() == Player)
        {
            Unit->bHasMoved = false;
            Unit->bHasAttacked = false;
        }
    }
}





void AAWGameMode::EndGame()
{
    bIsGameOver = true;

    bool bHumanUnitsRemaining = false;
    bool bAIUnitsRemaining = false;

    // Itera sulla mappa delle unit� per verificare se ci sono unit� per ciascun giocatore.
    for (auto& UnitPair : GField->GameUnitMap)
    {
        if (UnitPair.Value)
        {
            if (UnitPair.Value->GetPlayerOwner() == 0)
            {
                bHumanUnitsRemaining = true;
            }
            else if (UnitPair.Value->GetPlayerOwner() == 1)
            {
                bAIUnitsRemaining = true;
            }
        }
    }

    // Assumendo che l'array dei player sia memorizzato in "Players"
    for (AActor* PlayerActor : Players)
    {


        // Puoi anche aggiornare il messaggio sullo schermo usando il GameInstance, ad esempio:
        UAWGameInstance* GI = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
        if (GI)
        {
            if (bHumanUnitsRemaining && !bAIUnitsRemaining)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1,
                    10.f,
                    FColor::Green,
                    TEXT("HUMAN WINS!"),
                    true,
                    FVector2D(2.f, 2.f) // Scala pi� grande
                );
            }
            else if (!bHumanUnitsRemaining && bAIUnitsRemaining)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1,             // key
                    10.f,           // durata
                    FColor::Green,  // colore
                    TEXT("AI WINS!"),
                    /*bNewerOnTop*/ true,
                    FVector2D(2.f, 2.f)  // scala (x, y)
                );
            }



        }
    }
}

