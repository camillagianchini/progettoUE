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

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);

	GameInstance = nullptr;

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
}

void AHumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AHumanPlayer::OnTurn()
{
	IsMyTurn = true;
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("Human Turn"));
	}
}


//C'è bisogno del doppio click veloce per posizionare la seconda unità e per muovere e attaccare, non sono riuscita a trovare una soluzione
void AHumanPlayer::OnClick()
{

	AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM)
	{
		//UE_LOG(LogTemp, Error, TEXT("OnClick: GameMode not found"));
		return;
	}

	if (GM->bIsGameOver)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Game over: No more moves allowed"));
		return;
	}

	if (GM->CurrentPlayer != 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Not human player's turn."));
		return;
	}

	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("Human Turn"));
	}

	FVector2D RawTilePos = GetClickedTilePosition();
	FVector2D TilePos(FMath::FloorToFloat(RawTilePos.X), FMath::FloorToFloat(RawTilePos.Y));

	ATile* ClickedTile = nullptr;
	if (GM->GField && GM->GField->TileMap.Contains(TilePos))
	{
		ClickedTile = GM->GField->TileMap[TilePos];
	}
	if (!ClickedTile)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Tile not found at position: %s"), *TilePos.ToString());
		return;
	}

	if (GM->CurrentPhase == EGamePhase::Placement)
	{
		if (ClickedTile->GetTileStatus() != ETileStatus::EMPTY)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Tile %s is not empty."), *TilePos.ToString());
			return;
		}

		UWorld* World = GetWorld();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GM;
		FVector SpawnLocation = GM->GField->GetRelativePositionByXYPosition(TilePos.X, TilePos.Y);
		SpawnLocation.Z += 5.0f;
		bool bPlacedUnit = false;

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
					//UE_LOG(LogTemp, Warning, TEXT("Placed human Sniper at %s"), *TilePos.ToString());
					bPlacedUnit = true;
				}
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("HPSniperClass not assigned!"));
			}
		}
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
					//UE_LOG(LogTemp, Warning, TEXT("Placed human Brawler at %s"), *TilePos.ToString());
					bPlacedUnit = true;
				}
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("HPBrawlerClass not assigned!"));
			}
		}

	 if (bPlacedUnit)
		{
			GM->GField->ResetGameStatusField();
			GM->SelectedUnit = nullptr;
			GM->NextTurn();
		}
		return;
	}

	if (!GameMode->SelectedUnit)
	{
		if (ClickedTile->GetTileStatus() == ETileStatus::OCCUPIED)
		{
			AGameUnit* Unit = ClickedTile->GetGameUnit();
			if (Unit && Unit->GetPlayerOwner() == 0 && !(Unit->bHasMoved && Unit->bHasAttacked))
			{
				GameMode->SelectedUnit = Unit;
				ClickedTile->SetTileGameStatus(ETileGameStatus::SELECTED);
				ClickedTile->SetTileMaterial();
				GameMode->GField->ShowLegalMovesForUnit(Unit);
				//UE_LOG(LogTemp, Log, TEXT("Selected unit with ID %d for movement."), Unit->GetGameUnitID());
			}
		}
		return;
	}
	else
	{
		if (ClickedTile->GetGameUnit() == GameMode->SelectedUnit)
		{
			GameMode->SelectedUnit = nullptr;
			GameMode->GField->ResetGameStatusField();
			//UE_LOG(LogTemp, Log, TEXT("Deselected unit."));
			return;
		}
	}

	AGameUnit* SelectedUnit = GameMode->SelectedUnit;
	if (!SelectedUnit->bHasMoved)
	{
		if (ClickedTile->GetTileGameStatus() == ETileGameStatus::LEGAL_MOVE)
		{
			GameMode->GField->MoveUnit(SelectedUnit, TilePos, [this, SelectedUnit, TilePos]()
				{
					SelectedUnit->bHasMoved = true;

					//UE_LOG(LogTemp, Log, TEXT("Unit ID=%d moved to X=%.0f Y=%.0f"), SelectedUnit->GetGameUnitID(), TilePos.X, TilePos.Y);
					GameMode->GField->ResetGameStatusField();

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
						//UE_LOG(LogTemp, Log, TEXT("Showing attack options for unit ID=%d"), SelectedUnit->GetGameUnitID());
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

	if (SelectedUnit->bHasMoved && !SelectedUnit->bHasAttacked)
	{
		if (ClickedTile->GetTileGameStatus() == ETileGameStatus::CAN_ATTACK)
		{
			GameMode->GField->AttackUnit(SelectedUnit, TilePos);
			SelectedUnit->bHasAttacked = true;
			//UE_LOG(LogTemp, Log, TEXT("Unit ID=%d attacked at %s"), SelectedUnit->GetGameUnitID(), *TilePos.ToString());
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

	for (TActorIterator<AGameUnit> It(GetWorld()); It; ++It)
	{
		TraceParams.AddIgnoredActor(*It);
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return FVector2D::ZeroVector;
	}

	float MouseX, MouseY;
	PC->GetMousePosition(MouseX, MouseY);

	FVector WorldLocation, WorldDirection;
	if (PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
	{
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
		if (Unit
			&& Unit->GetPlayerOwner() == 0
			&& !Unit->IsDead()                               
			&& !(Unit->bHasMoved && Unit->bHasAttacked))
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
		//UE_LOG(LogTemp, Log, TEXT("Selecting unit ID=%d for the next action"), NextUnit->GetGameUnitID());
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			//UE_LOG(LogTemp, Log, TEXT("DoNextUnitAction: FlushPressedKeys called."));
			PC->FlushPressedKeys();
		}
	}
}