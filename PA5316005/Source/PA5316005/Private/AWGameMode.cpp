#include "AWGameMode.h"
#include "GameField.h"
#include "AWPlayerController.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AAWGameMode::AAWGameMode()
{
    // Imposta il PlayerController e il Pawn di default
    PlayerControllerClass = AAWPlayerController::StaticClass();
    DefaultPawnClass = AHumanPlayer::StaticClass();

    // Inizializza lo stato di gioco
    CurrentPlayer = 0;      // Di default il giocatore 0 (Human) inizia
    FieldSize = 25;         // Griglia 25x25

    // Inizializza le mappe per il posizionamento delle unità per i due giocatori (0 e 1)
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
}

void AAWGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Ottieni il riferimento al HumanPlayer (Pawn del primo PlayerController)
    AHumanPlayer* HumanPlayer = GetWorld()->GetFirstPlayerController()->GetPawn<AHumanPlayer>();
    if (!IsValid(HumanPlayer))
    {
        UE_LOG(LogTemp, Error, TEXT("No player of type '%s' was found."), *AHumanPlayer::StaticClass()->GetName());
        return;
    }

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
    float Zposition = 4000.0f; // Regola in base alla tua scena
    FVector CameraPos(CameraPosX, CameraPosX, Zposition);
    FRotator CameraRot(-90.0f, 0.0f, 0.0f);
    HumanPlayer->SetActorLocationAndRotation(CameraPos, CameraRot);

    // Aggiungi i giocatori: HumanPlayer (indice 0) e AI (indice 1)
    Players.Add(HumanPlayer);
    PlayerNames.Add(0, "Player");

    auto* IA = GetWorld()->SpawnActor<ARandomPlayer>(FVector::ZeroVector, FRotator::ZeroRotator);
    Players.Add(IA);
    PlayerNames.Add(1, "IA");

    // Esegui il coin toss per decidere chi inizia a posizionare le unità
    CoinTossForStartingPlayer();
}

void AAWGameMode::CoinTossForStartingPlayer()
{
    // Genera un numero casuale 0 o 1 per decidere chi inizia
    int32 CoinResult = FMath::RandRange(0, 1);
    CurrentPlayer = CoinResult;
    UE_LOG(LogTemp, Log, TEXT("Coin toss result: %d. Starting player is: %d"), CoinResult, CurrentPlayer);

    // Avvia la fase di posizionamento
    ChoosePlayerAndStartGame();
}

void AAWGameMode::ChoosePlayerAndStartGame()
{
    // Inizia la fase di posizionamento chiamando PlaceUnitForCurrentPlayer()
    PlaceUnitForCurrentPlayer();
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

            // Determina quale unità posizionare per l'AI
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
            SpawnLocation.Z += 50.0f;

            if (bPlaceSniper)
            {
                if (AISniperClass) // Usa il blueprint assegnato
                {
                    World->SpawnActor<ASniper>(AISniperClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    bSniperPlaced[1] = true;
                    UE_LOG(LogTemp, Log, TEXT("AI ha piazzato uno Sniper in %s"), *Position.ToString());
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
                    World->SpawnActor<ABrawler>(AIBrawlerClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    bBrawlerPlaced[1] = true;
                    UE_LOG(LogTemp, Log, TEXT("AI ha piazzato un Brawler in %s"), *Position.ToString());
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
        CurrentPlayer = 0;
    }
    else // Caso Human: il posizionamento umano viene gestito tramite l'input (click) in AHumanPlayer::OnClick()
    {
        // L'input del giocatore umano verrà gestito nel metodo OnClick()
    }

    // Controlla se la fase di posizionamento è terminata
    if (bSniperPlaced[0] && bBrawlerPlaced[0] && bSniperPlaced[1] && bBrawlerPlaced[1])
    {
        // Tutte le unità sono state posizionate: passa alla fase di battaglia
        CurrentPhase = EGamePhase::Battle;
        UE_LOG(LogTemp, Log, TEXT("Tutte le unità posizionate. Passaggio alla fase di battaglia."));
    }
    else
    {
        // Se non sono terminate, se il turno corrente è AI puoi chiamare nuovamente PlaceUnitForCurrentPlayer()
        // oppure attendere l'input umano.
        if (CurrentPlayer == 1)
        {
            PlaceUnitForCurrentPlayer();
        }
    }
}


void AAWGameMode::NextTurn()
{
    // Funzione da chiamare durante la fase Battle per gestire il cambio turno
    if (CurrentPhase == EGamePhase::Battle)
    {
        if (TutteLeUnitaHannoAgito(CurrentPlayer))
        {
            // Verifica le condizioni di vittoria
            if (CondizioniDiVittoria())
            {
                EndGame();
                return;
            }
            // Alterna il giocatore corrente
            CurrentPlayer = (CurrentPlayer == 0) ? 1 : 0;
            ResetActionsForPlayer(CurrentPlayer);

            // Aggiorna eventualmente il messaggio del turno (ad es. tramite il GameInstance)
            UAWGameInstance* GameInstance = Cast<UAWGameInstance>(GetGameInstance());
             if(GameInstance)
             {
                 GameInstance->SetTurnMessage((CurrentPlayer == 0) ? TEXT("Human Turn") : TEXT("AI Turn"));
            }
        }
    }
}

// Le seguenti funzioni sono stub da completare secondo la logica del tuo gioco

bool AAWGameMode::TutteLeUnitaHannoAgito(int32 Player)
{
    // Implementa il controllo se tutte le unità del giocatore "Player" hanno già agito in questo turno
    return true;
}

bool AAWGameMode::CondizioniDiVittoria()
{
    // Implementa la logica per verificare se un giocatore ha vinto (ad esempio, tutte le unità avversarie eliminate)
    return false;
}

void AAWGameMode::ResetActionsForPlayer(int32 Player)
{
    // Resetta lo stato delle unità del giocatore per permettere l'azione nel nuovo turno
}

void AAWGameMode::EndGame()
{
    bIsGameOver = true;
    UE_LOG(LogTemp, Log, TEXT("Gioco terminato!"));
}


bool AAWGameMode::DoMove(FVector2D Destination, bool bTestOnly)
{
    // Esempio di logica:
    // - Se bTestOnly == true, controlli solo se la mossa è valida
    // - Altrimenti, sposti effettivamente l'unità
    UE_LOG(LogTemp, Log, TEXT("DoMove chiamato con Destination=%s, bTestOnly=%s"),
        *Destination.ToString(), bTestOnly ? TEXT("true") : TEXT("false"));

    // Restituisci true se la mossa è valida, false altrimenti
    return true;
}

void AAWGameMode::TurnNextPlayer()
{
    UE_LOG(LogTemp, Warning, TEXT("TurnNextPlayer() stub called."));
    // Implementa la logica per passare al prossimo giocatore, se ti serve
}

void AAWGameMode::SetSelectedTile(FVector2D Position)
{
    UE_LOG(LogTemp, Warning, TEXT("SetSelectedTile() stub called with Pos: %s"), *Position.ToString());
    // Se la logica di selezione tile è altrove, puoi lasciarlo vuoto
}

void AAWGameMode::PlaceUnit(int32 Player, FVector2D Position)
{
    UE_LOG(LogTemp, Warning, TEXT("PlaceUnit() stub called for Player %d at %s"), Player, *Position.ToString());
    // Qui potresti effettivamente spawnare l'unità, se serve
}

void AAWGameMode::DoAttack(FVector2D AttackerPosition, FVector2D TargetPosition)
{
    UE_LOG(LogTemp, Warning, TEXT("DoAttack() stub called from %s to %s"), *AttackerPosition.ToString(), *TargetPosition.ToString());
    // Implementa qui la logica d'attacco (danno, calcolo contrattacco, ecc.)
}










