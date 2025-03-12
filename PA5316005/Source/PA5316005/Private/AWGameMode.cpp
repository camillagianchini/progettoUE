#include "AWGameMode.h"
#include "Sniper.h"
#include "Brawler.h"
#include "GameField.h"
#include "AWPlayerController.h"
#include "HumanPlayer.h"
#include "CoinTossWidget.h"
#include "RandomPlayer.h"
//#include "MovesPanel.h"
#include "EngineUtils.h"


AAWGameMode::AAWGameMode()
{

    PlayerControllerClass = AAWPlayerController::StaticClass();
    DefaultPawnClass = AHumanPlayer::StaticClass();

    // Inizializza lo stato di gioco
    
    CurrentPlayer = 0;    // Di default il giocatore 0 inizia
    
    FieldSize = 25;       // Griglia 25x25

    // Inizializza le mappe per il posizionamento delle unità
    // Assumiamo 2 giocatori: 0 e 1
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

    // Carica il blueprint del MovesPanel, se presente
    //static ConstructorHelpers::FClassFinder<UMovesPanel> PanelWidgetBPClass(TEXT("/Game/Blueprints/BP_MovesPanel"));
    //if (PanelWidgetBPClass.Succeeded())
    //{
        //PanelWidgetClass = PanelWidgetBPClass.Class;
    //}
    //else
    //{
        //UE_LOG(LogTemp, Warning, TEXT("BP_MovesPanel non trovato!"));
   // }
}

void AAWGameMode::BeginPlay()
{
    Super::BeginPlay();

    bIsGameOver = false;
    MoveCounter = 0;

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

    // Calcola la posizione della camera
    float CameraPosX = ((GField->TileSize * FieldSize) + ((FieldSize - 1) * GField->TileSize * GField->CellPadding)) * 0.5f;
    float Zposition = 4000.0f; // Adatta questo valore in base alla tua scena
    FVector CameraPos(CameraPosX, CameraPosX, Zposition);

    // Imposta una rotazione che punta verso il basso
    FRotator CameraRot(-90.0f, 0.0f, 0.0f);

    HumanPlayer->SetActorLocationAndRotation(CameraPos, CameraRot);

    // Human player = 0
    Players.Add(HumanPlayer);
    PlayerNames.Add(0, "Player");

    UGameInstance* GameInstance = Cast<UGameInstance>(GetGameInstance());

    auto* IA = GetWorld()->SpawnActor<ARandomPlayer>(FVector(), FRotator());
    Players.Add(IA);
    PlayerNames.Add(1, "IA");



    // Esegui il lancio della moneta per decidere chi inizia a posizionare le unità
    CoinTossForStartingPlayer();

    // Avvia la fase iniziale in cui i giocatori posizionano le loro unità
    //MovesPanel = CreateWidget<UMovesPanel>(GetGameInstance(), PanelWidgetClass);

    //if (MovesPanel)
    //{
        //MovesPanel->AddToViewport(0);
    //}


    
}

void AAWGameMode::CoinTossForStartingPlayer()
{
    // Genera un numero casuale 0 o 1
    int32 CoinResult = FMath::RandRange(0, 1);

    // Imposta il giocatore corrente in base al risultato
    CurrentPlayer = CoinResult;

    UE_LOG(LogTemp, Log, TEXT("Coin toss result: %d. Starting player is: %d"), CoinResult, CurrentPlayer);

    // Se hai il widget per il coin toss, crealo e aggiungilo al viewport
    if (CoinTossWidgetClass)
    {
        UCoinTossWidget* CoinTossWidget = CreateWidget<UCoinTossWidget>(GetWorld(), CoinTossWidgetClass);
        if (CoinTossWidget)
        {
            FText ResultText = (CurrentPlayer == 0) ? FText::FromString("Human") : FText::FromString("AI");
            CoinTossWidget->SetResultText(ResultText);
            CoinTossWidget->AddToViewport();
            UE_LOG(LogTemp, Log, TEXT("CoinTossWidget creato e aggiunto al viewport con il risultato: %s"), *ResultText.ToString());
        }
    }

    // Log extra per il passaggio di fase
     //UE_LOG(LogTemp, Log, TEXT("Coin toss completato, ora passo alla fase di posizionamento unità.")); 

    // Ora chiama il metodo che dà il via al posizionamento
    ChoosePlayerAndStartGame();
}

void AAWGameMode::ChoosePlayerAndStartGame()
{
    //UE_LOG(LogTemp, Log, TEXT("Il giocatore %d inizierà a piazzare le unità."));

    // Se tocca all'IA piazzare subito:
    if (CurrentPlayer == 1)
    {
        if (ARandomPlayer* IAPlayer = Cast<ARandomPlayer>(Players[1]))
        {
            IAPlayer->PlaceUnitsRandomly();
            // Dopo il piazzamento dell'IA, passa il turno al giocatore umano:
            TurnNextPlayer();
        }
    }
    else
    {
        // Tocca al giocatore umano:
        // Puoi mostrare un messaggio o attivare un widget per informare l'utente che è il suo turno.
        UE_LOG(LogTemp, Error, TEXT("ATTENDI INPUT CLICK"));
        // Qui non chiami PlaceUnit: il HumanPlayer, attraverso il suo sistema di input, gestirà i clic sulle tile
        // e chiamerà PlaceUnit (via HandleTileClick/HandlePlacementClick) quando il giocatore clicca una tile.
    }
}



void AAWGameMode::SetSelectedTile(const FVector2D Position) const
{
    // Reset the field colors
    GField->ResetGameStatusField();

    // Show selected Tile (blue color)
    GField->SelectTile(Position);

    // Return the moves for the piece at the given position passed as a parameter
    GField->SetLegalMoves(GField->LegalMoves(Position));

    // Show legal moves (yellow color)
    GField->ShowLegalMovesInTheField();
}

int32 AAWGameMode::GetNextPlayer(int32 Player) const
{
    Player++;
    if (!Players.IsValidIndex(Player))
    {
        Player = 0;
    }
    return Player;
}



void AAWGameMode::PlaceUnit(int32 PlayerID, FVector2D TilePosition, EGameUnitType UnitType)
{
    // Verifica che il GameField esista e che la posizione sia valida
    if (!GField || !GField->IsValidPosition(TilePosition))
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaceUnit: Posizione non valida %s"), *TilePosition.ToString());
        return;
    }

    // Verifica che la tile nella posizione indicata sia vuota
    ATile* SelectedTile = nullptr;
    if (GField->TileMap.Contains(TilePosition))
    {
        SelectedTile = GField->TileMap[TilePosition];
    }
    if (!SelectedTile || SelectedTile->GetTileStatus() != ETileStatus::EMPTY)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaceUnit: La tile %s non è disponibile"), *TilePosition.ToString());
        return;
    }

    // In base al tipo, controlla se l'unità è già stata posizionata per il giocatore
    if (UnitType == EGameUnitType::SNIPER)
    {
        bool bAlreadyPlaced = false;
        if (bSniperPlaced.Contains(PlayerID))
        {
            bAlreadyPlaced = bSniperPlaced[PlayerID];
        }
        if (bAlreadyPlaced)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlaceUnit: Lo Sniper per il giocatore %d è già stato posizionato"), PlayerID);
            return;
        }
        // Spawn dello Sniper
        GField->GenerateGameUnit<ASniper>(TilePosition, PlayerID);
        bSniperPlaced.Add(PlayerID, true);
    }
    else if (UnitType == EGameUnitType::BRAWLER)
    {
        bool bAlreadyPlaced = false;
        if (bBrawlerPlaced.Contains(PlayerID))
        {
            bAlreadyPlaced = bBrawlerPlaced[PlayerID];
        }
        if (bAlreadyPlaced)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlaceUnit: Il Brawler per il giocatore %d è già stato posizionato"), PlayerID);
            return;
        }
        // Spawn del Brawler
        GField->GenerateGameUnit<ABrawler>(TilePosition, PlayerID);
        bBrawlerPlaced.Add(PlayerID, true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaceUnit: Tipo unità non riconosciuto"));
        return;
    }

    // Log per confermare il posizionamento
    UE_LOG(LogTemp, Log, TEXT("PlaceUnit: Giocatore %d ha posizionato un'unità di tipo %s in %s"),
        PlayerID,
        (UnitType == EGameUnitType::SNIPER ? TEXT("SNIPER") : TEXT("BRAWLER")),
        *TilePosition.ToString());

    // Se necessario, controlla se tutti i giocatori hanno completato il posizionamento.
    // Ad esempio, per 2 giocatori, verifica che ciascuno abbia posizionato sia lo Sniper che il Brawler.
    bool bAllPlayersDone = false;
    if (bSniperPlaced.Contains(0) && bSniperPlaced[0] &&
        bBrawlerPlaced.Contains(0) && bBrawlerPlaced[0] &&
        bSniperPlaced.Contains(1) && bSniperPlaced[1] &&
        bBrawlerPlaced.Contains(1) && bBrawlerPlaced[1])
    {
        bAllPlayersDone = true;
    }
}

void AAWGameMode::TurnNextPlayer()
{
    MoveCounter += 1;
    CurrentPlayer = GetNextPlayer(CurrentPlayer);
    Players[CurrentPlayer]->OnTurn();
}

void AAWGameMode::DoMove(const FVector2D EndPosition, bool bIsGameMove)
{
    // Recupera la tile di partenza dal GameField
    FVector2D StartPos = GField->GetSelectedTile();
    if (StartPos.X < 0 || StartPos.Y < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DoMove: Nessuna tile selezionata."));
        return;
    }

    // Recupera la tile di destinazione e controlla che sia vuota
    ATile* EndTile = nullptr;
    if (GField->TileMap.Contains(EndPosition))
    {
        EndTile = GField->TileMap[EndPosition];
    }
    if (!EndTile || EndTile->GetTileStatus() != ETileStatus::EMPTY)
    {
        UE_LOG(LogTemp, Warning, TEXT("DoMove: La tile di destinazione %s non è vuota."), *EndPosition.ToString());
        return;
    }

    // Recupera la tile di partenza e l'unità in essa contenuta
    ATile* StartTile = nullptr;
    if (GField->TileMap.Contains(StartPos))
    {
        StartTile = GField->TileMap[StartPos];
    }
    if (!StartTile || !StartTile->GetGameUnit())
    {
        UE_LOG(LogTemp, Warning, TEXT("DoMove: Nessuna unità trovata sulla tile di partenza."));
        return;
    }
    AGameUnit* MovingUnit = StartTile->GetGameUnit();

    // Aggiorna lo stato della griglia:
    // - La tile di partenza diventa vuota
    // - La tile di destinazione viene occupata dall'unità in movimento
    SetTileMapStatus(StartPos, EndPosition);

    // Aggiorna la posizione logica dell'unità in movimento
    MovingUnit->SetGridPosition(EndPosition.X, EndPosition.Y);

    // Se la mossa fa parte della logica di gioco, registra la mossa nello storico
    if (bIsGameMove)
    {
        int32 UnitID = MovingUnit->GetGameUnitID();
        FMove NewMove(MoveCounter, UnitID, StartPos, EndPosition, -1);
        Moves.Add(NewMove);
        MoveCounter++;
    }

    // Resetta lo stato visivo del GameField
    GField->ResetGameStatusField();

    // Calcola le possibili mosse di attacco per l'unità spostata
    TArray<FVector2D> AttackMoves = MovingUnit->CalculateAttackMoves();

    if (AttackMoves.Num() > 0)
    {
        // Se ci sono mosse di attacco, evidenzia le tile d'attacco
        GField->SetLegalMoves(AttackMoves);
        GField->ShowLegalMovesInTheField();
        UE_LOG(LogTemp, Log, TEXT("DoMove: Attacco disponibile; attendere input per l'attacco."));
        // Non chiamiamo TurnNextPlayer() qui, perché il turno non termina finché non viene scelta l'azione d'attacco
    }
    else
    {
        // Se non sono disponibili attacchi, termina il turno
        TurnNextPlayer();
    }
}


void AAWGameMode::DoAttack(const FVector2D TargetPosition, bool bIsGameMove)
{
    // Recupera la tile di partenza (la tile selezionata)
    FVector2D StartPos = GField->GetSelectedTile();
    if (StartPos.X < 0 || StartPos.Y < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DoAttack: Nessuna tile selezionata per l'attacco."));
        return;
    }

    // Recupera la tile di partenza e l'unità attaccante
    ATile* StartTile = nullptr;
    if (GField->TileMap.Contains(StartPos))
    {
        StartTile = GField->TileMap[StartPos];
    }
    if (!StartTile || !StartTile->GetGameUnit())
    {
        UE_LOG(LogTemp, Warning, TEXT("DoAttack: Nessuna unità attaccante trovata sulla tile di partenza."));
        return;
    }
    AGameUnit* Attacker = StartTile->GetGameUnit();

    // Recupera la tile target e controlla che sia occupata
    ATile* TargetTile = nullptr;
    if (GField->TileMap.Contains(TargetPosition))
    {
        TargetTile = GField->TileMap[TargetPosition];
    }
    if (!TargetTile || TargetTile->GetTileStatus() != ETileStatus::OCCUPIED)
    {
        UE_LOG(LogTemp, Warning, TEXT("DoAttack: La tile target %s non è occupata."), *TargetPosition.ToString());
        return;
    }

    // Verifica che l'unità nella tile target appartenga all'avversario
    if (TargetTile->GetTileOwner() == Attacker->GetPlayerOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("DoAttack: La tile target %s contiene un'unità alleata."), *TargetPosition.ToString());
        return;
    }

    // Recupera l'unità bersaglio
    AGameUnit* TargetUnit = TargetTile->GetGameUnit();
    

    // Calcola il danno: un valore casuale tra DamageMin e DamageMax dell'attaccante
    int32 Damage = FMath::RandRange(Attacker->GetDamageMin(), Attacker->GetDamageMax());
    TargetUnit->TakeDamageUnit(Damage);
    UE_LOG(LogTemp, Log, TEXT("DoAttack: L'unità attaccante (ID: %d) infligge %d danni all'unità (ID: %d) in %s."),
        Attacker->GetGameUnitID(), Damage, TargetUnit->GetGameUnitID(), *TargetPosition.ToString());

    // Variabile per memorizzare l'ID dell'unità catturata, se presente
    int32 CapturedUnitID = -1;
    if (TargetUnit->IsDead())
    {
        CapturedUnitID = TargetUnit->GetGameUnitID();
        // Elimina l'unità bersaglio e aggiorna la tile target
        TargetUnit->Destroy();
        TargetTile->SetTileStatus(TargetTile->GetTileOwner(), ETileStatus::EMPTY, nullptr);
        UE_LOG(LogTemp, Log, TEXT("DoAttack: L'unità bersaglio (ID: %d) è stata eliminata."), CapturedUnitID);
    }

    // Se l'attaccante è uno Sniper, gestisci il contrattacco
    if (Attacker->GetGameUnitType() == EGameUnitType::SNIPER)
    {
        ASniper* Sniper = Cast<ASniper>(Attacker);
        if (Sniper)
        {
            Sniper->HandleCounterAttack(TargetUnit);
        }
    }

    // Se la mossa fa parte della logica di gioco, registra la mossa nello storico
    if (bIsGameMove)
    {
        int32 AttackerID = Attacker->GetGameUnitID();
        FMove NewMove(MoveCounter, AttackerID, StartPos, TargetPosition, CapturedUnitID);
        Moves.Add(NewMove);
        MoveCounter++;
    }

    // Resetta lo stato visivo del GameField (ad esempio, deseleziona le tile e ripristina i colori)
    GField->ResetGameStatusField();

    // Passa il turno al giocatore successivo
    TurnNextPlayer();
}


void AAWGameMode::SetTileMapStatus(const FVector2D Start, const FVector2D End) const
{
    ATile* StartTile = *GField->TileMap.Find(Start);
    // Rinomina la variabile locale da StartOwner a TileOwner per evitare conflitti
    const int32 TileOwner = StartTile->GetTileOwner();
    AGameUnit* GameUnit = StartTile->GetGameUnit();

    GameUnit->SetGridPosition(End.X, End.Y);

    StartTile->SetTileStatus(-1, ETileStatus::EMPTY, nullptr);

    // Usa TileOwner qui
    (*GField->TileMap.Find(End))->SetTileStatus(TileOwner, ETileStatus::OCCUPIED, GameUnit);
}


bool AAWGameMode::IsIllegalMove() const
{
    // Se il GameField non è valido, consideriamo la mossa illegale
    if (!GField)
    {
        UE_LOG(LogTemp, Warning, TEXT("IsIllegalMove: GField è nullo."));
        return true;
    }

    // Recupera la tile selezionata
    FVector2D SelectedTile = GField->GetSelectedTile();
    if (SelectedTile.X < 0 || SelectedTile.Y < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("IsIllegalMove: Nessuna tile selezionata."));
        return true;
    }

    // Recupera la tile corrispondente e l'unità presente
    ATile* Tile = nullptr;
    if (GField->TileMap.Contains(SelectedTile))
    {
        Tile = GField->TileMap[SelectedTile];
    }
 
    // Calcola le mosse legali per l'unità sulla tile selezionata
    TArray<FVector2D> LegalMoves = Tile->GetGameUnit()->CalculateLegalMoves();

    // Se non ci sono mosse legali, consideriamo la mossa illegale
    return (LegalMoves.Num() == 0);
}

bool AAWGameMode::CheckVictoryCondition() const
{
    int32 CountPlayer0 = 0;
    int32 CountPlayer1 = 0;

    // Itera su tutte le unità attualmente presenti nel GameUnitMap
    for (const auto& Pair : GameUnitMap)
    {
        if (Pair.Value) // Se l'unità è valida
        {
            int32 UnitOwner = Pair.Value->GetPlayerOwner();
            if (UnitOwner == 0)
            {
                CountPlayer0++;
            }
            else if (UnitOwner == 1)
            {
                CountPlayer1++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CheckVictoryCondition: Player0 units = %d, Player1 units = %d"), CountPlayer0, CountPlayer1);

    // Se uno dei due giocatori non ha più unità, la condizione di vittoria è soddisfatta.
    return (CountPlayer0 == 0 || CountPlayer1 == 0);
}

















