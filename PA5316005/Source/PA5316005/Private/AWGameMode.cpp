#include "AWGameMode.h"
#include "GameField.h"
#include "Tile.h"
#include "Sniper.h"
#include "Brawler.h"
#include "GameUnit.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AAWGameMode::AAWGameMode()
{
	// Inizializzazione di default
	MaxUnitsPerPlayer = 2;
	HumanUnitsPlaced = 0;
	AIUnitsPlaced = 0;
	bHumanStartsPlacement = FMath::RandBool();
	bIsHumanPlacementTurn = bHumanStartsPlacement;
	// Durante la fase di turno, chi inizia è colui che ha vinto il lancio della moneta
	bIsHumanTurn = bHumanStartsPlacement;
	CurrentPhase = EGamePhase::Placement;
}

void AAWGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Spawna il GameField
	SpawnGameField();

	// Mostra il risultato del lancio della moneta
	if (GEngine)
	{
		FString TossMsg = bHumanStartsPlacement ? TEXT("Coin Toss: Human starts placement") : TEXT("Coin Toss: AI starts placement");
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TossMsg);
	}

	// Inizia la fase di posizionamento
	StartUnitPlacement();
}

void AAWGameMode::SpawnGameField()
{
	if (GameFieldClass)
	{
		FVector SpawnLocation = FVector::ZeroVector;
		FRotator SpawnRotation = FRotator::ZeroRotator;
		GField = GetWorld()->SpawnActor<AGameField>(GameFieldClass, SpawnLocation, SpawnRotation);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameFieldClass is not set in AWGameMode!"));
	}
}

void AAWGameMode::StartUnitPlacement()
{
	// Mostra un messaggio per indicare il turno per il posizionamento
	if (GEngine)
	{
		FString TurnMsg = bIsHumanPlacementTurn ? TEXT("Human: Place your unit") : TEXT("AI: Place your unit");
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TurnMsg);
	}
	// La logica per attivare l'input per il posizionamento verrà gestita dalle classi player
}

void AAWGameMode::PlaceUnitOnTile(const FVector2D& TilePosition, bool bIsHumanPlayer, FName UnitType)
{
	if (!GField)
		return;

	ATile* TargetTile = GField->TileMap.FindRef(TilePosition);
	if (!TargetTile || TargetTile->GetTileStatus() != ETileStatus::EMPTY)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot place unit on tile (%f, %f): tile not empty"), TilePosition.X, TilePosition.Y);
		return;
	}

	AGameUnit* SpawnedUnit = nullptr;
	if (UnitType == "Sniper")
	{
		SpawnedUnit = GetWorld()->SpawnActor<ASniper>(
			ASniper::StaticClass(),
			GField->GetRelativeLocationByXYPosition(TilePosition.X, TilePosition.Y),
			FRotator::ZeroRotator
		);
	}
	else if (UnitType == "Brawler")
	{
		SpawnedUnit = GetWorld()->SpawnActor<ABrawler>(
			ABrawler::StaticClass(),
			GField->GetRelativeLocationByXYPosition(TilePosition.X, TilePosition.Y),
			FRotator::ZeroRotator
		);
	}

	if (SpawnedUnit)
	{
		// Imposta le proprietà dell'unità
		SpawnedUnit->GridPosition = TilePosition;
		SpawnedUnit->UnitOwner = bIsHumanPlayer ? 1 : 2;
		SpawnedUnit->GameFieldRef = GField;

		// Aggiorna la tile come occupata
		TargetTile->SetTileStatus(bIsHumanPlayer ? 1 : 2, ETileStatus::OCCUPIED);

		// Registra l'unità nel GameMode
		RegisterUnit(SpawnedUnit);

		// Aggiorna il conteggio di unità posizionate
		if (bIsHumanPlayer)
			HumanUnitsPlaced++;
		else
			AIUnitsPlaced++;

		if (GEngine)
		{
			FString LogMsg = FString::Printf(TEXT("%s placed %s at (%f, %f)"),
				bIsHumanPlayer ? TEXT("Human") : TEXT("AI"),
				*UnitType.ToString(),
				TilePosition.X, TilePosition.Y);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LogMsg);
		}
	}

	// Alterna il turno di posizionamento se non tutte le unità sono state piazzate
	if (HumanUnitsPlaced < MaxUnitsPerPlayer || AIUnitsPlaced < MaxUnitsPerPlayer)
	{
		bIsHumanPlacementTurn = !bIsHumanPlacementTurn;
		if (GEngine)
		{
			FString NextMsg = bIsHumanPlacementTurn ? TEXT("Human: Place your unit") : TEXT("AI: Place your unit");
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, NextMsg);
		}
	}
	else
	{
		// Tutte le unità sono state piazzate: passa alla fase di Turn
		CurrentPhase = EGamePhase::Turn;
		// Il giocatore che inizia il turno è colui che ha vinto il lancio di moneta
		bIsHumanTurn = bHumanStartsPlacement;
		if (GEngine)
		{
			FString TurnMsg = bIsHumanTurn ? TEXT("Human Turn") : TEXT("AI Turn");
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TurnMsg);
		}
	}
}

void AAWGameMode::StartTurn(bool bIsHumanTurnInput)
{
	bIsHumanTurn = bIsHumanTurnInput;
	if (GEngine)
	{
		FString TurnMsg = bIsHumanTurn ? TEXT("Human Turn") : TEXT("AI Turn");
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TurnMsg);
	}
	// Qui potresti abilitare l'input sul Pawn o invocare OnTurn() del rispettivo player.
}

void AAWGameMode::EndTurn()
{
	// Alterna il turno
	bIsHumanTurn = !bIsHumanTurn;
	if (GEngine)
	{
		FString TurnMsg = bIsHumanTurn ? TEXT("Human Turn") : TEXT("AI Turn");
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TurnMsg);
	}
	CheckGameEnd();
}

void AAWGameMode::CheckGameEnd()
{
	// Controlla quante unità vive rimangono per ciascun giocatore
	int32 HumanUnitsAlive = 0;
	int32 AIUnitsAlive = 0;

	for (AGameUnit* Unit : AllUnits)
	{
		if (IsValid(Unit) && Unit->HitPoints > 0)
		{
			if (Unit->UnitOwner == 1)
				HumanUnitsAlive++;
			else if (Unit->UnitOwner == 2)
				AIUnitsAlive++;
		}
	}

	if (HumanUnitsAlive == 0)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI Wins!"));
		// Puoi disabilitare ulteriori input o notificare la fine partita
		return;
	}

	if (AIUnitsAlive == 0)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Human Wins!"));
		return;
	}
}

void AAWGameMode::LogMove(const FString& MoveEntry)
{
	UE_LOG(LogTemp, Log, TEXT("Move: %s"), *MoveEntry);
}

void AAWGameMode::SetSelectedTile(const FVector2D& TilePosition)
{
	if (GField)
	{
		GField->SetSelectedTile(TilePosition);
	}
}

void AAWGameMode::RegisterUnit(AGameUnit* NewUnit)
{
	if (NewUnit)
	{
		AllUnits.Add(NewUnit);
	}
}



