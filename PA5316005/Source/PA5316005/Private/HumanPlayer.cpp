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

// IPlayerInterface
void AHumanPlayer::OnTurn()
{
    IsMyTurn = true;
    if (GameInstance)
    {
        GameInstance->SetTurnMessage(TEXT("Human Turn!"));
    }
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

    // Verifica che sia il turno dell'umano (player 0)
    if (GM->CurrentPlayer != 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Non è il turno dell'umano."));
        return;
    }

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

    // --- Fase di Battaglia ---
    // Se una unità è già selezionata, interpreta il click come comando per muovere o attaccare.
    if (GM->SelectedUnit)
    {

        ETileGameStatus TileGS = ClickedTile->GetTileGameStatus();
        // Se la tile è un movimento legale e l'unità non ha ancora mosso
        if (TileGS == ETileGameStatus::LEGAL_MOVE)
        {
            GM->GField->MoveUnit(GM->SelectedUnit, TilePosition);
            GM->SelectedUnit->bHasMoved = true;
            GM->GField->ResetGameStatusField();
            GM->SelectedUnit = nullptr;
            DoNextUnitAction();
            return;
        }
        else if (TileGS == ETileGameStatus::CAN_ATTACK)
        {
            GM->GField->AttackUnit(GM->SelectedUnit, TilePosition);
            GM->SelectedUnit->bHasAttacked = true;
            GM->GField->ResetGameStatusField();
            GM->SelectedUnit = nullptr;
            DoNextUnitAction();
            return;
        }
        // Se il click è su una tile occupata (per cambiare selezione)
        if (ClickedTile->GetTileStatus() == ETileStatus::OCCUPIED)
        {
            AGameUnit* NewUnit = ClickedTile->GetGameUnit();
            if (NewUnit && NewUnit->GetPlayerOwner() == 0 &&
                (!NewUnit->bHasMoved || !NewUnit->bHasAttacked))
            {
                GM->SelectedUnit = NewUnit;
                UE_LOG(LogTemp, Log, TEXT("Nuova unità selezionata sulla tile %s."), *TilePosition.ToString());
                GM->GField->ShowLegalMovesForUnit(NewUnit);
                return;
            }
        }
    }
    else // Nessuna unità selezionata: seleziona quella sulla tile se appartiene al giocatore e non ha agito
    {
        if(ClickedTile->GetTileStatus() == ETileStatus::OCCUPIED)
        {
            AGameUnit* Unit = ClickedTile->GetGameUnit();
            if (Unit && Unit->GetPlayerOwner() == 0 &&
                (!Unit->bHasMoved || !Unit->bHasAttacked))
            {
                GM->SelectedUnit = Unit;
                UE_LOG(LogTemp, Log, TEXT("Unità selezionata sulla tile %s."), *TilePosition.ToString());
                GM->GField->ShowLegalMovesForUnit(Unit);
                return;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("L'unità sulla tile ha già completato le azioni."));
            }
        }
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
    // Recupera il GameMode e il GameField
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        UE_LOG(LogTemp, Warning, TEXT("DoNextUnitAction: GameMode o GameField non trovati."));
        return;
    }

    // Cerca la prima unità appartenente all'umano (PlayerOwner == 0) che non ha ancora agito
    for (auto& Pair : GM->GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;
        if (Unit && Unit->GetPlayerOwner() == 0 && !(Unit->bHasMoved && Unit->bHasAttacked))
        {
            // Se la trovi, la imposti come unità selezionata e ne evidenzi le mosse legali
            GM->SelectedUnit = Unit;
            GM->GField->ShowLegalMovesForUnit(Unit);
            UE_LOG(LogTemp, Log, TEXT("DoNextUnitAction: Selezionata unità ID %d"), Unit->GetGameUnitID());
            return;
        }
    }

    // Se non trovi nessuna unità, significa che tutte hanno agito: passa il turno all'AI
    UE_LOG(LogTemp, Warning, TEXT("DoNextUnitAction: Tutte le unità umane hanno agito. Passaggio del turno all'AI."));
    GM->NextTurn();
}




































