#include "HumanPlayer.h"
#include "AWGameMode.h"
#include "AWGameInstance.h"
#include "Tile.h"
#include "GameUnit.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AHumanPlayer::AHumanPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Crea e imposta la Camera come RootComponent
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);

	// Inizializza variabili
	bIsMyTurn = false;
	SelectedUnit = nullptr;

	// Di default, piazziamo un "Sniper" (puoi cambiarlo a "Brawler" in editor)
	UnitToPlace = "Sniper";
}

void AHumanPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Ottieni riferimenti a GameMode e GameInstance
	GameModeRef = Cast<AAWGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	GameInstanceRef = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}

void AHumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// IPlayerInterface
void AHumanPlayer::OnTurn()
{
	bIsMyTurn = true;

	if (GameInstanceRef)
	{
		GameInstanceRef->SetTurnMessage(TEXT("Human Turn!"));
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Your Turn!"));
	}
}

void AHumanPlayer::OnWin()
{
	if (GameInstanceRef)
	{
		GameInstanceRef->SetTurnMessage(TEXT("Human Wins!"));
		GameInstanceRef->IncrementScoreHumanPlayer();
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("You Win!"));
	}
}

void AHumanPlayer::OnLose()
{
	if (GameInstanceRef)
	{
		GameInstanceRef->SetTurnMessage(TEXT("Human Loses!"));
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("You Lose!"));
	}
}

void AHumanPlayer::MakeMove()
{
	// Per il giocatore umano, la logica di �fare la mossa� � attivata via OnClick.
	// Puoi lasciare questo metodo vuoto oppure aggiungere logica extra (es. countdown).
}

void AHumanPlayer::OnClick()
{
	// Se non c�� GameModeRef, o se la fase non � gestita, esci
	if (!GameModeRef)
		return;

	// Esegui un trace sotto il cursore
	FHitResult Hit;
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
	{
		ATile* ClickedTile = Cast<ATile>(Hit.GetActor());
		AGameUnit* ClickedUnit = Cast<AGameUnit>(Hit.GetActor());

		// Controlla la fase di gioco
		switch (GameModeRef->CurrentPhase)
		{
		case EGamePhase::Placement:
		{
			// Se � il turno di posizionamento dell'umano
			if (GameModeRef->bIsHumanPlacementTurn)
			{
				if (ClickedTile)
				{
					HandlePlacementClick(ClickedTile);
				}
			}
			break;
		}
		case EGamePhase::Turn:
		{
			// Se � il turno dell'umano
			if (bIsMyTurn)
			{
				if (ClickedTile)
				{
					HandleTileClick(ClickedTile);
				}
				else if (ClickedUnit)
				{
					HandleUnitClick(ClickedUnit);
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

void AHumanPlayer::HandlePlacementClick(ATile* ClickedTile)
{
	if (!ClickedTile || !GameModeRef)
		return;

	// Chiediamo al GameMode di piazzare l'unit�
	GameModeRef->PlaceUnitOnTile(ClickedTile->GetGridPosition(), true, UnitToPlace);

	// Se vuoi alternare tra Sniper e Brawler, puoi farlo qui
	// Esempio: se avevi piazzato uno Sniper, la prossima volta piazzi un Brawler
	// Oppure puoi usare una UI per selezionare l�unit� da piazzare
}

void AHumanPlayer::HandleTileClick(ATile* ClickedTile)
{
	if (!ClickedTile || !GameModeRef)
		return;

	// Esempio minimal: Se abbiamo un�unit� selezionata, proviamo a spostarla su questa tile se � vuota
	if (SelectedUnit)
	{
		if (ClickedTile->GetTileStatus() == ETileStatus::EMPTY)
		{
			// Sposta l�unit�
			SelectedUnit->MoveUnit(ClickedTile->GetGridPosition());
			// Esempio: termina immediatamente il turno dopo un movimento
			bIsMyTurn = false;
			GameModeRef->EndTurn();
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Tile not empty!"));
			}
		}
	}
	else
	{
		// Nessuna unit� selezionata: potresti ignorare il click o deselezionare
		GameModeRef->SetSelectedTile(ClickedTile->GetGridPosition());
	}
}

void AHumanPlayer::HandleUnitClick(AGameUnit* ClickedUnit)
{
	if (!ClickedUnit)
		return;

	// Se l�unit� cliccata � di propriet� del giocatore umano
	if (ClickedUnit->UnitOwner == 1)
	{
		// Selezioniamo quest�unit�
		SelectedUnit = ClickedUnit;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
				FString::Printf(TEXT("Selected your unit: %s"), *ClickedUnit->GetName()));
		}
	}
	else
	{
		// L�unit� cliccata � avversaria, se abbiamo un�unit� selezionata potremmo attaccare
		if (SelectedUnit)
		{
			SelectedUnit->AttackUnit(ClickedUnit);
			// Termina il turno dopo l�attacco (esempio minimal)
			bIsMyTurn = false;
			GameModeRef->EndTurn();
		}
	}
}




