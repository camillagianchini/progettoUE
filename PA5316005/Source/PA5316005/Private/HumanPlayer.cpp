#include "HumanPlayer.h"
#include "GameField.h"
#include "Tile.h"
#include "Sniper.h"
#include "Brawler.h"
#include "EngineUtils.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AHumanPlayer::AHumanPlayer()
	: GameMode(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessPlayer = EAutoReceiveInput::Player0;


	// Crea e imposta la Camera come RootComponent
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);

	GameInstance = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// Default init values
	PlayerNumber = 0;
}

void AHumanPlayer::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    UE_LOG(LogTemp, Warning, TEXT("HumanPlayer::BeginPlay - Pawn spawnato e posseduto?"));
}

void AHumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// IPlayerInterface
void AHumanPlayer::OnTurn()
{
	IsMyTurn = true;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Your Turn"));
	GameInstance->SetTurnMessage(TEXT("Human Turn!"));
}

void AHumanPlayer::OnWin()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("You Win!"));
	GameInstance->SetTurnMessage(TEXT("Human Wins!"));
	GameInstance->IncrementScoreHumanPlayer();
}

void AHumanPlayer::OnLose()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("You Lose!"));
	GameInstance->SetTurnMessage(TEXT("Human Loses!"));
}

void AHumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AHumanPlayer::OnClick()
{
    UE_LOG(LogTemp, Warning, TEXT("AHumanPlayer::OnClick() chiamato!"));

    // Ottieni il riferimento al GameMode
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
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
    UE_LOG(LogTemp, Log, TEXT("Posizione cliccata (raw): %s"), *RawTilePosition.ToString());

    // Converti in coordinate intere per farle corrispondere alle chiavi della TileMap
    FVector2D TilePosition = FVector2D(FMath::FloorToFloat(RawTilePosition.X), FMath::FloorToFloat(RawTilePosition.Y));
    UE_LOG(LogTemp, Log, TEXT("Posizione cliccata (arrotondata): %s"), *TilePosition.ToString());

    // Recupera la tile dal GameField
    ATile* ClickedTile = nullptr;
    if (GM->GField && GM->GField->GetTileMap().Contains(TilePosition))
    {
        ClickedTile = GM->GField->GetTileMap()[TilePosition];
    }
    if (!ClickedTile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tile non trovata per la posizione: %s"), *TilePosition.ToString());
        return;
    }

    // --- Fase Iniziale: Posizionamento Unitá ---
    // Se non siamo ancora in fase di battaglia, il click serve per piazzare l'unitá sulla tile libera.
    if (GM->CurrentPhase != EGamePhase::Battle)
    {
        // Controlla che la tile sia libera
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
                    // Aggiorna lo stato della tile: ora è occupata dalla nuova unità
                    ClickedTile->SetTileStatus(0, ETileStatus::OCCUPIED, SpawnedUnit);
                    UE_LOG(LogTemp, Warning, TEXT("TileOwner: %d, TileStatus: %d"), ClickedTile->GetTileOwner(), (int32)ClickedTile->GetTileStatus());
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
                    ClickedTile->SetTileStatus(0, ETileStatus::OCCUPIED, SpawnedUnit);
                    UE_LOG(LogTemp, Warning, TEXT("TileOwner: %d, TileStatus: %d"), ClickedTile->GetTileOwner(), (int32)ClickedTile->GetTileStatus());
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

    if (ClickedTile)
    {
        UE_LOG(LogTemp, Log, TEXT("Tile trovata in %s, stato: %d"), *TilePosition.ToString(), (int32)ClickedTile->GetTileStatus());
        UE_LOG(LogTemp, Log, TEXT("Tile trovata in %s, stato: %s"), *TilePosition.ToString(), *ClickedTile->GameStatusToString());

    }

    // --- Fase di Battaglia: Selezione Unitá ---
    // Se siamo in fase di battaglia, ci aspettiamo che il click sia su una tile occupata da una tua unitá.
    if (ClickedTile->GetTileStatus() == ETileStatus::OCCUPIED)
    {
        AGameUnit* Unit = ClickedTile->GetGameUnit();
        if (Unit && Unit->GetPlayerOwner() == 0)
        {
            // Se esiste già un'unitá selezionata e diversa, deseleziona quella precedente
            if (GM->SelectedUnit && GM->SelectedUnit != Unit)
            {
                UE_LOG(LogTemp, Log, TEXT("Deselezione dell'unità precedente."));
                // Puoi implementare qui la logica per deselezionare (ad esempio, ripristinando lo stato della tile)
            }
            ClickedTile->SetTileGameStatus(ETileGameStatus::SELECTED);
            GM->SelectedUnit = Unit;
            UE_LOG(LogTemp, Log, TEXT("Unità selezionata sulla tile %s"), *TilePosition.ToString());
            ShowLegalMovesForUnit(Unit);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("La tile cliccata non contiene una unità di proprietà del giocatore."));
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

void AHumanPlayer::ShowLegalMovesForUnit(AGameUnit* Unit)
{
    if (!Unit)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShowLegalMovesForUnit: Unità nulla."));
        return;
    }

    // Ottieni il riferimento al GameMode e al GameField
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShowLegalMovesForUnit: GameMode o GameField non trovato."));
        return;
    }

    // Reset delle tile precedenti
    GM->GField->ResetGameStatusField();

    // Calcola le mosse legali per il movimento (puoi anche usare una funzione dedicata che filtra i movimenti validi)
    FVector2D StartPos = Unit->GetGridPosition();
    TArray<FVector2D> LegalMoves = Unit->CalculateLegalMoves();

    // Filtra le mosse in base alla validità della tile (es. fuori mappa, tile occupata, ecc.)
    TArray<FVector2D> ValidMoves;
    for (const FVector2D& MovePos : LegalMoves)
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

    // Evidenzia le tile valide per il movimento impostando lo stato su LEGAL_MOVE
    for (const FVector2D& MovePos : ValidMoves)
    {
        if (GM->GField->IsValidPosition(MovePos))
        {
            ATile* Tile = GM->GField->GetTileMap()[MovePos];
            if (Tile)
            {
                Tile->SetTileGameStatus(ETileGameStatus::LEGAL_MOVE);
            }
        }
    }

    // Calcola le possibili mosse di attacco
    TArray<FVector2D> AttackMoves = Unit->CalculateAttackMoves();
    TArray<FVector2D> ValidAttacks;
    for (const FVector2D& AttackPos : AttackMoves)
    {
        if (GM->GField->IsValidPosition(AttackPos))
        {
            ATile* Tile = GM->GField->GetTileMap()[AttackPos];
            // Considera l'attacco solo se la tile è occupata da un nemico
            if (Tile && Tile->GetTileStatus() == ETileStatus::OCCUPIED)
            {
                AGameUnit* Target = Tile->GetGameUnit();
                if (Target && Target->GetPlayerOwner() != Unit->GetPlayerOwner())
                {
                    ValidAttacks.Add(AttackPos);
                }
            }
        }
    }

    // Evidenzia le tile valide per l'attacco impostando lo stato su CAN_ATTACK
    for (const FVector2D& AttackPos : ValidAttacks)
    {
        if (GM->GField->IsValidPosition(AttackPos))
        {
            ATile* Tile = GM->GField->GetTileMap()[AttackPos];
            if (Tile)
            {
                Tile->SetTileGameStatus(ETileGameStatus::CAN_ATTACK);
            }
        }
    }
}




