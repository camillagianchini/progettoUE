#include "AWGameMode.h"
#include "GameField.h"
#include "Tile.h"
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
    float Zposition = 3500.0f; // Regola in base alla tua scena
    FVector CameraPos(CameraPosX, CameraPosX, Zposition);
    FRotator CameraRot(-90.0f, 0.0f, 0.0f);
    HumanPlayer->SetActorLocationAndRotation(CameraPos, CameraRot);

    // Aggiungi i giocatori: HumanPlayer (indice 0) e AI (indice 1)
    Players.Add(HumanPlayer);
    PlayerNames.Add(0, "Player");

    auto* IA = GetWorld()->SpawnActor<ARandomPlayer>(FVector::ZeroVector, FRotator::ZeroRotator);
    Players.Add(IA);
    PlayerNames.Add(1, "IA");

    // Esegui il coin toss per decidere chi inizia a posizionare le unit�
    CoinTossForStartingPlayer();
}

void AAWGameMode::CoinTossForStartingPlayer()
{
    // Genera un numero casuale 0 o 1 per decidere chi inizia
    int32 CoinResult = FMath::RandRange(0, 1);
    StartingPlayer = CoinResult; // salva in una variabile di AAWGameMode
    CurrentPlayer = CoinResult;
    UE_LOG(LogTemp, Log, TEXT("Coin toss result: %d. Starting player is: %d"), CoinResult, CurrentPlayer);

    // Avvia la fase di posizionamento
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
        CurrentPlayer = 0;
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


void AAWGameMode::NextTurn()
{
    CurrentPlayer = 1 - CurrentPlayer;

    // Resetto le azioni per il nuovo player
    ResetActionsForPlayer(CurrentPlayer);

    // Log di debug
    UE_LOG(LogTemp, Warning, TEXT("NextTurn() -> adesso CurrentPlayer = %d"), CurrentPlayer);

    // Se AI => AI.OnTurn(), altrimenti umano
    if (CurrentPlayer == 1)
    {
        ARandomPlayer* AI = Cast<ARandomPlayer>(Players[1]);
        if (AI)
            AI->OnTurn();
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







bool AAWGameMode::CondizioniDiVittoria()
{
    // Implementa la logica per verificare se un giocatore ha vinto (ad esempio, tutte le unit� avversarie eliminate)
    return false;
}



















