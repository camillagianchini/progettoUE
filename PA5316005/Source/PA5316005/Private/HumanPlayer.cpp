#include "HumanPlayer.h"
#include "AWGameMode.h"
#include "AWGameInstance.h"
#include "GameField.h"
#include "Tile.h"
#include "Sniper.h"
#include "Brawler.h"
#include "EngineUtils.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

AHumanPlayer::AHumanPlayer()
	: GameMode(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Crea e imposta la Camera come RootComponent
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);

	// I riferimenti verranno impostati in BeginPlay
	GameInstance = nullptr;

	// Inizialmente, il player umano ha numero 0
	PlayerNumber = 0;
	IsMyTurn = false;
}

void AHumanPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Recupera il GameMode (assicurati che il Pawn sia già posseduto)
	GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameInstance)
	{
		GameInstance = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	}
}

void AHumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// L'input per il click è gestito dal PlayerController EnhancedInput (la funzione OnClick() verrà chiamata da lì)
}

//////////////////////////////////////////////////////////////////////////
// IPlayerInterface
void AHumanPlayer::OnTurn()
{
	// Questo metodo viene chiamato dal GameMode quando il turno umano inizia
	IsMyTurn = true;
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("Human Turn"));
	}
}

void AHumanPlayer::OnWin()
{
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("Human Wins!"));
		
	}
}

void AHumanPlayer::OnLose()
{
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("Human Loses!"));
	}
}

//////////////////////////////////////////////////////////////////////////
// OnClick & Tile Interaction
void AHumanPlayer::OnClick()
{
	AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM)
	{
		UE_LOG(LogTemp, Error, TEXT("OnClick: GameMode non trovato"));
		return;
	}

	// Se non è il turno umano, ignora il click
	if (GM->CurrentPlayer != 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Non è il turno dell'umano."));
		return;
	}

	// Aggiorna il messaggio del turno
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("Human Turn"));
	}

	// Ottieni la posizione cliccata sulla griglia
	FVector2D RawTilePos = GetClickedTilePosition();
	FVector2D TilePos(FMath::FloorToFloat(RawTilePos.X), FMath::FloorToFloat(RawTilePos.Y));

	ATile* ClickedTile = nullptr;
	if (GM->GField && GM->GField->TileMap.Contains(TilePos))
	{
		ClickedTile = GM->GField->TileMap[TilePos];
	}
	if (!ClickedTile)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tile non trovata per la posizione: %s"), *TilePos.ToString());
		return;
	}

	// Fase di Placement: l'umano piazza le unità tramite click
	if (GM->CurrentPhase != EGamePhase::Battle)
	{
		if (ClickedTile->GetTileStatus() != ETileStatus::EMPTY)
		{
			UE_LOG(LogTemp, Warning, TEXT("La tile in %s non è libera."), *TilePos.ToString());
			return;
		}

		UWorld* World = GetWorld();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GM;
		FVector SpawnLocation = GM->GField->GetRelativePositionByXYPosition(TilePos.X, TilePos.Y);
		SpawnLocation.Z += 5.0f; // Offset per far apparire l'unità sopra la griglia
		bool bPlacedUnit = false;

		// Se non è già piazzato lo Sniper umano, piazzalo
		if (!GM->bSniperPlaced.FindRef(0))
		{
			if (GM->HPSniperClass)
			{
				ASniper* SpawnedUnit = World->SpawnActor<ASniper>(GM->HPSniperClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				if (SpawnedUnit)
				{
					GM->bSniperPlaced[0] = true;
					SpawnedUnit->SetPlayerOwner(0);
					SpawnedUnit->SetGameUnitID();
					SpawnedUnit->SetGridPosition(TilePos.X, TilePos.Y);
					ClickedTile->SetTileStatus(0, ETileStatus::OCCUPIED, SpawnedUnit);
					int32 NewUnitKey = GM->GField->GameUnitMap.Num();
					GM->GField->GameUnitMap.Add(NewUnitKey, SpawnedUnit);
					UE_LOG(LogTemp, Warning, TEXT("Piazzato Sniper umano in %s"), *TilePos.ToString());
					bPlacedUnit = true;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HPSniperClass non assegnato!"));
			}
		}
		// Se lo Sniper è già piazzato, piazza il Brawler umano
		else if (!GM->bBrawlerPlaced.FindRef(0))
		{
			if (GM->HPBrawlerClass)
			{
				ABrawler* SpawnedUnit = World->SpawnActor<ABrawler>(GM->HPBrawlerClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				if (SpawnedUnit)
				{
					GM->bBrawlerPlaced[0] = true;
					SpawnedUnit->SetPlayerOwner(0);
					SpawnedUnit->SetGameUnitID();
					SpawnedUnit->SetGridPosition(TilePos.X, TilePos.Y);
					ClickedTile->SetTileStatus(0, ETileStatus::OCCUPIED, SpawnedUnit);
					int32 NewUnitKey = GM->GField->GameUnitMap.Num();
					GM->GField->GameUnitMap.Add(NewUnitKey, SpawnedUnit);
					UE_LOG(LogTemp, Warning, TEXT("Piazzato Brawler umano in %s"), *TilePos.ToString());
					bPlacedUnit = true;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HPBrawlerClass non assegnato!"));
			}
		}

		// Se un'unità è stata piazzata, controlla se tutte le unità sono state posizionate
		if (bPlacedUnit)
		{
			bool bHumanDone = GM->bSniperPlaced.FindRef(0) && GM->bBrawlerPlaced.FindRef(0);
			bool bAIDone = GM->bSniperPlaced.FindRef(1) && GM->bBrawlerPlaced.FindRef(1);
			if (bHumanDone && bAIDone)
			{
				GM->CurrentPhase = EGamePhase::Battle;
				UE_LOG(LogTemp, Log, TEXT("Tutte le unità posizionate. Passaggio alla fase di battaglia."));
				// Imposta il CurrentPlayer sul valore del coin toss (StartingPlayer)
				GM->CurrentPlayer = GM->StartingPlayer;
				GM->NextTurn();
			}
			else
			{
				// Passa il turno al posizionamento dell'AI
				UE_LOG(LogTemp, Log, TEXT("Passaggio del turno all'AI (placement)."));
				GM->NextTurn();
			}
		}

		return;
	}

	// Fase di Battle: gestisci la selezione, il movimento e l'attacco
	if (!GameMode->SelectedUnit)
	{
		// Se clicco su una tile occupata da una mia unità che non ha ancora agito, selezionala
		if (ClickedTile->GetTileStatus() == ETileStatus::OCCUPIED)
		{
			AGameUnit* Unit = ClickedTile->GetGameUnit();
			if (Unit && Unit->GetPlayerOwner() == 0 && !(Unit->bHasMoved && Unit->bHasAttacked))
			{
				GameMode->SelectedUnit = Unit;
				ClickedTile->SetTileGameStatus(ETileGameStatus::SELECTED);
				ClickedTile->SetTileMaterial();
				GameMode->GField->ShowLegalMovesForUnit(Unit);
				UE_LOG(LogTemp, Log, TEXT("Unità selezionata con ID %d per movimento."), Unit->GetGameUnitID());
			}
		}
		return;
	}
	else // Se un'unità è già selezionata
	{
		// Se clicco nuovamente sulla stessa unità, deselezionala
		if (ClickedTile->GetGameUnit() == GameMode->SelectedUnit)
		{
			GameMode->SelectedUnit = nullptr;
			GameMode->GField->ResetGameStatusField();
			UE_LOG(LogTemp, Log, TEXT("Unità deselezionata."));
			return;
		}
	}

	AGameUnit* SelectedUnit = GameMode->SelectedUnit;
	// Se l'unità non ha ancora mosso, gestisci il movimento
	if (!SelectedUnit->bHasMoved)
	{
		if (ClickedTile->GetTileGameStatus() == ETileGameStatus::LEGAL_MOVE)
		{
			GameMode->GField->MoveUnit(SelectedUnit, TilePos, [this, SelectedUnit, TilePos]()
				{
					SelectedUnit->bHasMoved = true;
					UE_LOG(LogTemp, Log, TEXT("Unità ID=%d mossa in X=%.0f Y=%.0f"), SelectedUnit->GetGameUnitID(), TilePos.X, TilePos.Y);
					GameMode->GField->ResetGameStatusField();

					// Calcola le possibili celle d'attacco dalla nuova posizione
					TArray<FVector2D> AttackTiles = SelectedUnit->CalculateAttackMoves();
					bool bHasEnemy = false;
					for (const FVector2D& Pos : AttackTiles)
					{
						ATile* AttackTile = GameMode->GField->TileMap.Contains(Pos) ? GameMode->GField->TileMap[Pos] : nullptr;
						if (AttackTile && AttackTile->GetTileStatus() == ETileStatus::OCCUPIED)
						{
							AGameUnit* PotentialEnemy = AttackTile->GetGameUnit();
							if (PotentialEnemy && PotentialEnemy->GetPlayerOwner() != 0)
							{
								bHasEnemy = true;
								break;
							}
						}
					}
					if (bHasEnemy)
					{
						GameMode->GField->ShowLegalAttackOptionsForUnit(SelectedUnit);
						UE_LOG(LogTemp, Log, TEXT("Mostro opzioni di attacco per l'unità ID=%d"), SelectedUnit->GetGameUnitID());
					}
					else
					{
						SelectedUnit->bHasAttacked = true;
						GameMode->SelectedUnit = nullptr;
						DoNextUnitAction();
					}
				});
			return;
		}
	}

	// Se l'unità ha mosso e non ha ancora attaccato, gestisci l'attacco
	if (SelectedUnit->bHasMoved && !SelectedUnit->bHasAttacked)
	{
		if (ClickedTile->GetTileGameStatus() == ETileGameStatus::CAN_ATTACK)
		{
			GameMode->GField->AttackUnit(SelectedUnit, TilePos);
			SelectedUnit->bHasAttacked = true;
			UE_LOG(LogTemp, Log, TEXT("Unità ID=%d ha attaccato in %s"), SelectedUnit->GetGameUnitID(), *TilePos.ToString());
			GameMode->GField->ResetGameStatusField();
			GameMode->SelectedUnit = nullptr;
			DoNextUnitAction();
			return;
		}
	}

	GameMode->GField->ResetGameStatusField();
	if (SelectedUnit)
	{
		if (!SelectedUnit->bHasMoved)
			GameMode->GField->ShowLegalMovesForUnit(SelectedUnit);
		else if (!SelectedUnit->bHasAttacked)
			GameMode->GField->ShowLegalAttackOptionsForUnit(SelectedUnit);
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
	for (auto& Pair : GameMode->GField->GameUnitMap)
	{
		AGameUnit* Unit = Pair.Value;
		if (Unit && Unit->GetPlayerOwner() == 0 && !(Unit->bHasMoved && Unit->bHasAttacked))
		{
			NextUnit = Unit;
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