#include "HumanPlayer.h"
#include "AWGameMode.h"
#include "GameField.h"
#include "Tile.h"
#include "Sniper.h"
#include "Brawler.h"
#include "EngineUtils.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AWGameInstance.h"

AHumanPlayer::AHumanPlayer()
    : GameMode(nullptr)
{
    PrimaryActorTick.bCanEverTick = true;

    AutoPossessPlayer = EAutoReceiveInput::Player0;

    // Crea e imposta la Camera come RootComponent
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    SetRootComponent(Camera);

    // Sarà impostato in BeginPlay, se disponibile
    GameInstance = nullptr;

    // Default init values
    PlayerNumber = 0;
    IsMyTurn = false;
}

void AHumanPlayer::BeginPlay()
{
    Super::BeginPlay();

    GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GameInstance)
    {
        GameInstance = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    }

    //UE_LOG(LogTemp, Warning, TEXT("HumanPlayer::BeginPlay - Pawn spawnato e posseduto?"));
}

void AHumanPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}




void AHumanPlayer::OnWin()
{
    if (GameInstance)
    {
        GameInstance->SetTurnMessage(TEXT("Human Wins!"));
        GameInstance->IncrementScoreHumanPlayer();
    }
}

void AHumanPlayer::OnLose()
{
    if (GameInstance)
    {
        GameInstance->SetTurnMessage(TEXT("Human Loses!"));
    }
}

void AHumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AHumanPlayer::OnClick()
{


    // Ottieni il riferimento al GameMode
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    UE_LOG(LogTemp, Warning, TEXT("Turno corrente: %d"), GM ? GM->CurrentPlayer : -1);
    if (!GM)
    {
        UE_LOG(LogTemp, Error, TEXT("OnClick: GameMode non trovato"));
        return;
    }

    // Verifica che sia il turno dell'umano (Player 0)
    if (GM->CurrentPlayer != 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Non è il turno dell'umano."));
        return;
    }

    GameInstance->SetTurnMessage("Human Turn");


    // Ottieni la posizione della tile cliccata
    FVector2D RawTilePosition = GetClickedTilePosition();
    UE_LOG(LogTemp, Log, TEXT("Posizione cliccata: %s"), *RawTilePosition.ToString());

    // Converti in coordinate intere per farle corrispondere alle chiavi della TileMap
    FVector2D TilePosition(FMath::FloorToFloat(RawTilePosition.X), FMath::FloorToFloat(RawTilePosition.Y));

    // Recupera la tile dal GameField
    ATile* ClickedTile = nullptr;
    if (GM->GField && GM->GField->TileMap.Contains(TilePosition))
    {
        ClickedTile = GM->GField->TileMap[TilePosition];
    }
    if (!ClickedTile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tile non trovata per la posizione: %s"), *TilePosition.ToString());
        return;
    }

    // --- Fase di Posizionamento (non in Battle) ---
    if (GM->CurrentPhase != EGamePhase::Battle)
    {
        if (ClickedTile->GetTileStatus() != ETileStatus::EMPTY)
        {
            UE_LOG(LogTemp, Warning, TEXT("La tile in %s non è libera."), *TilePosition.ToString());
            return;
        }

        bool bPlacedUnit = false;
        UWorld* World = GetWorld();
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GM;
        FVector SpawnLocation = GM->GField->GetRelativePositionByXYPosition(TilePosition.X, TilePosition.Y);
        SpawnLocation.Z += 5.0f; // Offset per far apparire l'unità sopra la griglia

        // Decidi quale unità piazzare per il giocatore umano
        if (!GM->bSniperPlaced.FindRef(0))
        {
            if (GM->HPSniperClass)
            {
                ASniper* SpawnedUnit = World->SpawnActor<ASniper>(GM->HPSniperClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                if (SpawnedUnit)
                {
                    GM->bSniperPlaced.Add(0, true);
                    SpawnedUnit->SetPlayerOwner(0);
                    SpawnedUnit->SetGameUnitID();
                    SpawnedUnit->SetGridPosition(TilePosition.X, TilePosition.Y);
                    ClickedTile->SetTileStatus(0, ETileStatus::OCCUPIED, SpawnedUnit);
                    int32 NewUnitKey = GM->GField->GameUnitMap.Num();
                    GM->GField->GameUnitMap.Add(NewUnitKey, SpawnedUnit);
                    UE_LOG(LogTemp, Warning, TEXT("TileOwner: %d, TileStatus: %d"),
                        ClickedTile->GetTileOwner(), (int32)ClickedTile->GetTileStatus());
                    bPlacedUnit = true;
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("HPSniperClass non assegnato!"));
            }
        }
        else if (!GM->bBrawlerPlaced.FindRef(0))
        {
            if (GM->HPBrawlerClass)
            {
                ABrawler* SpawnedUnit = World->SpawnActor<ABrawler>(GM->HPBrawlerClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                if (SpawnedUnit)
                {
                    GM->bBrawlerPlaced.Add(0, true);
                    SpawnedUnit->SetPlayerOwner(0);
                    SpawnedUnit->SetGameUnitID();
                    SpawnedUnit->SetGridPosition(TilePosition.X, TilePosition.Y);
                    ClickedTile->SetTileStatus(0, ETileStatus::OCCUPIED, SpawnedUnit);
                    int32 NewUnitKey = GM->GField->GameUnitMap.Num();
                    GM->GField->GameUnitMap.Add(NewUnitKey, SpawnedUnit);
                    UE_LOG(LogTemp, Warning, TEXT("TileOwner: %d, TileStatus: %d"),
                        ClickedTile->GetTileOwner(), (int32)ClickedTile->GetTileStatus());
                    bPlacedUnit = true;
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("HPBrawlerClass non assegnato!"));
            }
        }

        if (bPlacedUnit)
        {
            bool bHumanDone = GM->bSniperPlaced.FindRef(0) && GM->bBrawlerPlaced.FindRef(0);
            bool bAllPlaced = bHumanDone && GM->bSniperPlaced.FindRef(1) && GM->bBrawlerPlaced.FindRef(1);
            if (bAllPlaced)
            {
                GM->CurrentPhase = EGamePhase::Battle;
                UE_LOG(LogTemp, Log, TEXT("Tutte le unità sono state posizionate. Passaggio alla fase di battaglia."));
            }
            else
            {
                GM->CurrentPlayer = 1;
                UE_LOG(LogTemp, Log, TEXT("Passaggio del turno all'AI."));
                GM->PlaceUnitForCurrentPlayer();
            }
        }
        return;
    }

    if (!GameMode->SelectedUnit)
    {
        // Se clicco su una tile occupata, seleziona l'unità se è mia e non ha già completato le azioni
        if (ClickedTile->GetTileStatus() == ETileStatus::OCCUPIED)
        {
            AGameUnit* Unit = ClickedTile->GetGameUnit();
            if (Unit && Unit->GetPlayerOwner() == 0 && !(Unit->bHasMoved && Unit->bHasAttacked))
            {
                GameMode->SelectedUnit = Unit;
                // Imposta la tile in cui si trova l'unità come SELECTED
                ClickedTile->SetTileGameStatus(ETileGameStatus::SELECTED);
                // Applica il materiale corrispondente (in questo caso blu)
                ClickedTile->SetTileMaterial();
                // Ora mostra le altre opzioni (ad es. le tile in cui l'unità può muoversi)
                GameMode->GField->ShowLegalMovesForUnit(Unit);
                UE_LOG(LogTemp, Log, TEXT("Unità selezionata con ID %d per movimento."), Unit->GetGameUnitID());
            }
        }


        return;
    }
    else // Abbiamo già un'unità selezionata
    {
        // Se clicco sulla stessa unità già selezionata, togli le evidenziazioni e deseleziona
        if (ClickedTile->GetGameUnit() == GM->SelectedUnit)
        {
            GM->SelectedUnit = nullptr;
            GM->GField->ResetGameStatusField();
            UE_LOG(LogTemp, Log, TEXT("Unità deselezionata."));
            return;
        }
    }

    // Abbiamo un'unità selezionata
    AGameUnit* SelectedUnit = GameMode->SelectedUnit;

    // ------------------ FASE DI MOVIMENTO ------------------
    if (!SelectedUnit->bHasMoved)
    {
        if (ClickedTile->GetTileGameStatus() == ETileGameStatus::LEGAL_MOVE)
        {
            // Utilizza MoveUnit con callback per attendere il completamento del movimento
            GameMode->GField->MoveUnit(SelectedUnit, TilePosition, [this, SelectedUnit, GM, TilePosition]()
                {
                    // Una volta completato il movimento, segnala che l'unità ha mosso
                    SelectedUnit->bHasMoved = true;
                    UE_LOG(LogTemp, Log, TEXT("Unità ID=%d mossa in X=%.0f Y=%.0f"),
                        SelectedUnit->GetGameUnitID(), TilePosition.X, TilePosition.Y);

                

                    // Resetta le evidenziazioni
                    GM->GField->ResetGameStatusField();

                    // Ricalcola le celle d'attacco partendo dalla nuova posizione
                    TArray<FVector2D> AttackTiles = SelectedUnit->CalculateAttackMoves();
                    bool bHasEnemyInRange = false;
                    for (const FVector2D& Pos : AttackTiles)
                    {
                        ATile* AttackTile = GM->GField->TileMap.Contains(Pos) ? GM->GField->TileMap[Pos] : nullptr;
                        if (AttackTile && AttackTile->GetTileStatus() == ETileStatus::OCCUPIED)
                        {
                            AGameUnit* PotentialEnemy = AttackTile->GetGameUnit();
                            if (PotentialEnemy && PotentialEnemy->GetPlayerOwner() != 0)
                            {
                                bHasEnemyInRange = true;
                                break;
                            }
                        }
                    }
                    if (bHasEnemyInRange)
                    {
                        // Evidenzia le opzioni di attacco se esiste almeno un nemico in range
                        GM->GField->ShowLegalAttackOptionsForUnit(SelectedUnit);
                        UE_LOG(LogTemp, Log, TEXT("Mostro opzioni di attacco per l'unità ID=%d"),
                            SelectedUnit->GetGameUnitID());
                    }
                    else
                    {
                        // Se non ci sono attacchi disponibili, l'unità conclude il turno
                        UE_LOG(LogTemp, Log, TEXT("Nessun attacco disponibile per l'unità ID=%d. Turno unità concluso."),
                            SelectedUnit->GetGameUnitID());
                        SelectedUnit->bHasAttacked = true;
                        GM->SelectedUnit = nullptr;
                        DoNextUnitAction();
                    }
                });
            return;
        }
    }

    // ------------------ FASE DI ATTACCO ------------------
    if (SelectedUnit->bHasMoved && !SelectedUnit->bHasAttacked)
    {
        if (ClickedTile->GetTileGameStatus() == ETileGameStatus::CAN_ATTACK)
        {
            GM->GField->AttackUnit(SelectedUnit, TilePosition);
            SelectedUnit->bHasAttacked = true;
            UE_LOG(LogTemp, Log, TEXT("Unità ID=%d ha attaccato in %s"),
                SelectedUnit->GetGameUnitID(), *TilePosition.ToString());
            GM->GField->ResetGameStatusField();
            GM->SelectedUnit = nullptr;
            DoNextUnitAction();
            return;
        }
    }

    // Se il click non corrisponde a nessuna azione valida, resetta le evidenziazioni
    GM->GField->ResetGameStatusField();
    if (SelectedUnit)
    {
        if (!SelectedUnit->bHasMoved)
            GM->GField->ShowLegalMovesForUnit(SelectedUnit);
        else if (!SelectedUnit->bHasAttacked)
            GM->GField->ShowLegalAttackOptionsForUnit(SelectedUnit);
    }
}





FVector2D AHumanPlayer::GetClickedTilePosition() const
{
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(this);

    // Ignora anche tutti gli attori di tipo AGameUnit (così il raycast non li colpisce)
    for (TActorIterator<AGameUnit> It(GetWorld()); It; ++It)
    {
        TraceParams.AddIgnoredActor(*It);
    }

    // Ottieni il PlayerController (assicurati che sia valido)
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return FVector2D::ZeroVector;
    }

    // Ottieni la posizione del mouse in screen space
    float MouseX, MouseY;
    PC->GetMousePosition(MouseX, MouseY);

    // Converti la posizione del mouse in world space usando DeprojectScreenPositionToWorld
    FVector WorldLocation, WorldDirection;
    if (PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
    {
        // Esegui un raycast per trovare il punto di impatto sul GameField
        FVector Start = WorldLocation;
        FVector End = Start + (WorldDirection * 10000.f);

        FHitResult HitResult;
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, TraceParams))
        {
            AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
            if (GM && GM->GField)
            {
                return GM->GField->GetXYPositionByRelativeLocation(HitResult.Location);
            }
        }
    }
    // Se qualcosa fallisce, ritorna (0,0)
    return FVector2D::ZeroVector;

}


void AHumanPlayer::DoNextUnitAction()
{
    if (!GameMode || !GameMode->GField)
        return;

    AGameUnit* NextUnit = nullptr;
    // Cerca tra le unità umane quella che non ha completato movimento e attacco
    for (auto& Pair : GameMode->GField->GameUnitMap)
    {
        AGameUnit* U = Pair.Value;
        // Usa IsValid() per controllare che l'unità sia valida
        if (IsValid(U) && U->GetPlayerOwner() == 0 && !(U->bHasMoved && U->bHasAttacked))
        {
            NextUnit = U;
            break;
        }
    }

    if (!NextUnit)
    {

        GameMode->NextTurn();
    }
    else
    {
        GameMode->SelectedUnit = NextUnit;
        GameMode->GField->ResetGameStatusField();
        GameMode->GField->ShowLegalMovesForUnit(NextUnit);
        UE_LOG(LogTemp, Log, TEXT("Seleziono unità ID=%d per la prossima azione"), NextUnit->GetGameUnitID());
    }
}










































