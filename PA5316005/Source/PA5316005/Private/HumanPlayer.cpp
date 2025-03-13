#include "HumanPlayer.h"
#include "GameField.h"
#include "Sniper.h"
#include "Brawler.h"
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
    FVector2D TilePosition = GetClickedTilePosition();

    // Recupera la tile dal GameField usando la TileMap
    ATile* ClickedTile = nullptr;
    if (GM->GField)
    {
        if (GM->GField->GetTileMap().Contains(TilePosition))
        {
            ClickedTile = GM->GField->GetTileMap()[TilePosition];
        }
    }

    if (!ClickedTile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tile non trovata per la posizione: %s"), *TilePosition.ToString());
        return;
    }

    // Controlla se la tile è libera
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
    SpawnLocation.Z += 50.0f; // Offset per far apparire l'unità sopra la griglia

    // Decidi quale unità piazzare per il giocatore umano (player 0)
    if (!GM->bSniperPlaced.FindRef(0))
    {
        // Spawna lo Sniper per il giocatore umano utilizzando il blueprint HPSniperClass
        if (GM->HPSniperClass)
        {
            World->SpawnActor<ASniper>(GM->HPSniperClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
            GM->bSniperPlaced.Add(0, true);
            UE_LOG(LogTemp, Log, TEXT("Human ha piazzato uno Sniper in %s"), *TilePosition.ToString());
            bPlacedUnit = true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("HPSniperClass non assegnato!"));
        }
    }
    else if (!GM->bBrawlerPlaced.FindRef(0))
    {
        // Spawna il Brawler per il giocatore umano utilizzando il blueprint HPBrawlerClass
        if (GM->HPBrawlerClass)
        {
            World->SpawnActor<ABrawler>(GM->HPBrawlerClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
            GM->bBrawlerPlaced.Add(0, true);
            UE_LOG(LogTemp, Log, TEXT("Human ha piazzato un Brawler in %s"), *TilePosition.ToString());
            bPlacedUnit = true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("HPBrawlerClass non assegnato!"));
        }
    }

    // Se un'unità è stata piazzata, passa il turno o avvia la fase di battaglia
    if (bPlacedUnit)
    {
        bool bHumanDone = GM->bSniperPlaced.FindRef(0) && GM->bBrawlerPlaced.FindRef(0);
        bool bAllPlaced = bHumanDone &&
            GM->bSniperPlaced.FindRef(1) && GM->bBrawlerPlaced.FindRef(1);
        if (bAllPlaced)
        {
            GM->CurrentPhase = EGamePhase::Battle;
            UE_LOG(LogTemp, Log, TEXT("Tutte le unità sono state posizionate. Passaggio alla fase di battaglia."));
        }
        else
        {
            // Passa il turno all'AI (player 1)
            GM->CurrentPlayer = 1;
            UE_LOG(LogTemp, Log, TEXT("Passaggio del turno all'AI."));
            GM->PlaceUnitForCurrentPlayer();
        }
    }
}

FVector2D AHumanPlayer::GetClickedTilePosition() const
{
    return FVector2D();
}





