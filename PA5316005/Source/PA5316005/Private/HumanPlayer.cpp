#include "HumanPlayer.h"
#include "GameField.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AHumanPlayer::AHumanPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Crea e imposta la Camera come RootComponent
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);

	GameInstance = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// Default init values
	PlayerNumber = -1;
}

void AHumanPlayer::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
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


void AHumanPlayer::OnClick()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnClick: PlayerController non trovato."));
		return;
	}

	// Ottieni la posizione del mouse sullo schermo
	float MouseX, MouseY;
	if (!PC->GetMousePosition(MouseX, MouseY))
	{
		UE_LOG(LogTemp, Warning, TEXT("OnClick: Impossibile ottenere la posizione del mouse."));
		return;
	}

	// Deproietta la posizione del mouse in una linea nel mondo
	FVector WorldLocation, WorldDirection;
	if (!PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
	{
		UE_LOG(LogTemp, Warning, TEXT("OnClick: Deprojection fallita."));
		return;
	}

	// Imposta l'inizio e la fine del trace (una lunghezza sufficiente, es. 10000 unit�)
	FVector TraceStart = WorldLocation;
	FVector TraceEnd = WorldLocation + WorldDirection * 10000.0f;

	// Esegui il line trace
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignora il pawn stesso

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			// Verifica se l'attore colpito � una Tile
			if (ATile* HitTile = Cast<ATile>(HitActor))
			{
				UE_LOG(LogTemp, Log, TEXT("OnClick: Tile colpita in %s."), *HitTile->GetGridPosition().ToString());
				HandleTileClick(HitTile);
				return;
			}
			// Se non � una Tile, verifica se � un'unit� di gioco
			if (AGameUnit* HitUnit = Cast<AGameUnit>(HitActor))
			{
				UE_LOG(LogTemp, Log, TEXT("OnClick: GameUnit colpita (ID: %d)."), HitUnit->GetGameUnitID());
				HandleGameUnitClick(HitUnit);
				return;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("OnClick: Nessun attore colpito dal line trace."));
	}
}

void AHumanPlayer::HandleTileClick(ATile* ClickedTile)
{
	if (!ClickedTile)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Tile cliccata in %s."), *ClickedTile->GetGridPosition().ToString());

	// Determina se siamo nella fase di posizionamento
	// Ad esempio, se il giocatore non ha ancora posizionato entrambe le unit� (Sniper e Brawler)
	bool bPlacementPhase = false;
	if (GameMode)
	{
		// Supponiamo che PlayerNumber sia stato impostato correttamente
		bool bSniperPlaced = false;
		bool bBrawlerPlaced = false;
		if (GameMode->bSniperPlaced.Contains(PlayerNumber))
		{
			bSniperPlaced = GameMode->bSniperPlaced[PlayerNumber];
		}
		if (GameMode->bBrawlerPlaced.Contains(PlayerNumber))
		{
			bBrawlerPlaced = GameMode->bBrawlerPlaced[PlayerNumber];
		}
		// Se almeno una unit� non � stata posizionata, siamo nella fase di posizionamento.
		bPlacementPhase = !(bSniperPlaced && bBrawlerPlaced);
	}

	if (bPlacementPhase)
	{
		// In fase di placement, gestisci il click come placement
		HandlePlacementClick(ClickedTile);
	}
	else
	{
		// Altrimenti, in turno, seleziona la tile e mostra le possibili mosse.
		if (GameMode)
		{
			GameMode->SetSelectedTile(ClickedTile->GetGridPosition());
		}
	}
}
void AHumanPlayer::HandlePlacementClick(ATile* ClickedTile)
{
	if (!ClickedTile)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Placement click sulla tile in %s."), *ClickedTile->GetGridPosition().ToString());

	if (GameMode)
	{
		// Usa PlayerNumber per identificare il giocatore corrente
		int32 PlayerID = PlayerNumber; // Assicurati che PlayerNumber sia impostato correttamente

		// Se lo Sniper non � ancora stato posizionato per questo giocatore, posizionalo
		bool bSniperAlreadyPlaced = false;
		if (GameMode->bSniperPlaced.Contains(PlayerID))
		{
			bSniperAlreadyPlaced = GameMode->bSniperPlaced[PlayerID];
		}
		if (!bSniperAlreadyPlaced)
		{
			GameMode->PlaceUnit(PlayerID, ClickedTile->GetGridPosition(), EGameUnitType::SNIPER);
		}
		// Altrimenti, se il Brawler non � ancora stato posizionato, posizionalo
		else
		{
			bool bBrawlerAlreadyPlaced = false;
			if (GameMode->bBrawlerPlaced.Contains(PlayerID))
			{
				bBrawlerAlreadyPlaced = GameMode->bBrawlerPlaced[PlayerID];
			}
			if (!bBrawlerAlreadyPlaced)
			{
				GameMode->PlaceUnit(PlayerID, ClickedTile->GetGridPosition(), EGameUnitType::BRAWLER);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HumanPlayer: Tutte le unit� sono gi� state posizionate per il giocatore %d."), PlayerID);
			}
		}
	}
}

void AHumanPlayer::HandleGameUnitClick(AGameUnit* ClickedUnit)
{
	if (!ClickedUnit)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Unit� cliccata (ID: %d) in posizione %s."),
		ClickedUnit->GetGameUnitID(), *ClickedUnit->GetGridPosition().ToString());

	if (GameMode)
	{
		// Se l'unit� cliccata appartiene al giocatore corrente, selezionala per il movimento
		if (ClickedUnit->GetPlayerOwner() == PlayerNumber)
		{
			GameMode->SetSelectedTile(ClickedUnit->GetGridPosition());
		}
		else
		{
			// Se l'unit� cliccata � nemica, otteniamo la tile corrispondente
			ATile* TargetTile = nullptr;
			if (GameMode->GField && GameMode->GField->TileMap.Contains(ClickedUnit->GetGridPosition()))
			{
				TargetTile = GameMode->GField->TileMap[ClickedUnit->GetGridPosition()];
			}

			if (TargetTile)
			{
				UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Unit� nemica cliccata. Iniziamo l'attacco."));
				ExecuteTheAttackForHumanPlayer(TargetTile);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HandleGameUnitClick: Impossibile trovare la tile corrispondente all'unit� cliccata."));
			}
		}
	}
}


void AHumanPlayer::ExecuteTheMoveForHumanPlayer(const ATile* EndTile)
{
	if (!EndTile)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteTheMoveForHumanPlayer: EndTile � nullo."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ExecuteTheMoveForHumanPlayer: Spostamento verso la tile in %s."), *EndTile->GetGridPosition().ToString());

	if (GameMode)
	{
		// Chiamata al GameMode per eseguire il movimento, passando la posizione della tile di destinazione e indicando che si tratta di una mossa di gioco
		GameMode->DoMove(EndTile->GetGridPosition(), true);
	}
}

void AHumanPlayer::ExecuteTheAttackForHumanPlayer(const ATile* TargetTile)
{
	if (!TargetTile)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteTheAttackForHumanPlayer: TargetTile is null."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Eseguo l'attacco sulla tile in %s."), *TargetTile->GetGridPosition().ToString());

	if (GameMode)
	{
		// Chiama il metodo DoAttack passando la posizione della tile target e indicando che si tratta di una mossa di gioco
		GameMode->DoAttack(TargetTile->GetGridPosition(), true);
	}
}
