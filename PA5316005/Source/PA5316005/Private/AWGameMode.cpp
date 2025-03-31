#include "AWGameMode.h"
#include "GameField.h"
#include "Tile.h"
#include "AWPlayerController.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "AStarPlayer.h"
#include "EngineUtils.h"
#include "UnitListWidget.h"
#include "CoinTossWidget.h"
#include "MovesPanel.h"
#include "OpponentSelectionWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AAWGameMode::AAWGameMode()
{
    PlayerControllerClass = AAWPlayerController::StaticClass();
    DefaultPawnClass = AHumanPlayer::StaticClass();

    CurrentPlayer = 0; 
    FieldSize = 25;        
    bFirstBattleTurn = false;

    bSniperPlaced.Add(0, false);
    bSniperPlaced.Add(1, false);
    bBrawlerPlaced.Add(0, false);
    bBrawlerPlaced.Add(1, false);

    bIsGameOver = false;
    MoveCounter = 0;
    CurrentPhase = EGamePhase::Placement;
    bIsAITurnInProgress = false;
}

void AAWGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (OpponentSelectionWidgetClass)
    {
        OpponentSelectionWidget = CreateWidget<UOpponentSelectionWidget>(GetWorld(), OpponentSelectionWidgetClass);
        if (OpponentSelectionWidget)
        {
            OpponentSelectionWidget->OnOpponentSelected.AddDynamic(this, &AAWGameMode::OnOpponentSelected);
            OpponentSelectionWidget->AddToViewport();

            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(OpponentSelectionWidget->TakeWidget());
            GetWorld()->GetFirstPlayerController()->SetInputMode(InputMode);
            GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;
        }
    }
    else
    {
        SpawnCoinTossWidget();
    }

    if (MovesPanelClass)
    {
        MovesPanel = CreateWidget<UMovesPanel>(GetWorld(), MovesPanelClass);
        if (MovesPanel)
        {
            MovesPanel->AddToViewport();
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(OpponentSelectionWidget->TakeWidget());
            GetWorld()->GetFirstPlayerController()->SetInputMode(InputMode);
            GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;
        }
    }
    
    AHumanPlayer* HumanPlayer = GetWorld()->GetFirstPlayerController()->GetPawn<AHumanPlayer>();
    if (!IsValid(HumanPlayer))
    {
        //UE_LOG(LogTemp, Error, TEXT("No player of type '%s' was found."), *AHumanPlayer::StaticClass()->GetName());
        return;
    }

    Players.Add(HumanPlayer);
    PlayerNames.Add(0, "Human");

    if (GameFieldClass != nullptr)
    {
        GField = GetWorld()->SpawnActor<AGameField>(GameFieldClass);
        GField->Size = FieldSize;
    }
    else
    {
        //UE_LOG(LogTemp, Error, TEXT("Game Field is null"));
    }

    float CameraPosX = ((GField->TileSize * FieldSize) + ((FieldSize - 1) * GField->TileSize * GField->CellPadding)) * 0.5f;
    float Zposition = 3500.0f;
    FVector CameraPos(CameraPosX, CameraPosX, Zposition);
    FRotator CameraRot(-90.0f, 0.0f, 0.0f);
    HumanPlayer->SetActorLocationAndRotation(CameraPos, CameraRot);
}

void AAWGameMode::OnOpponentSelected(int32 SelectedOpponent)
{
    UAWGameInstance* GI = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GI)
    {
        GI->SetOpponent(SelectedOpponent);
    }

    if (OpponentSelectionWidget)
    {
        OpponentSelectionWidget->RemoveFromParent();
        OpponentSelectionWidget = nullptr;
    }

    if (GI && GI->OpponentType == 1) 
    {
        auto* IA = GetWorld()->SpawnActor<AAStarPlayer>(FVector::ZeroVector, FRotator::ZeroRotator);
        Players.Add(IA);
        PlayerNames.Add(1, "IA");
    }
    else
    {
        auto* IA = GetWorld()->SpawnActor<ARandomPlayer>(FVector::ZeroVector, FRotator::ZeroRotator);
        Players.Add(IA);
        PlayerNames.Add(1, "IA");
    }

    SpawnCoinTossWidget();

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this, GI]()
        {
            if (UnitListWidgetClass)
            {
                UUnitListWidget* UnitList = CreateWidget<UUnitListWidget>(GetWorld(), UnitListWidgetClass);
                if (UnitList)
                {
                    EOpponentType OppType = (GI->OpponentType == 1) ? EOpponentType::AStar : EOpponentType::Random;
                    UnitList->SetOpponentType(OppType);

                    UnitList->AddToViewport();

                    UnitListWidget = UnitList;
                }
            }
        }, 2.5f, false);
}


void AAWGameMode::SpawnCoinTossWidget()
{
    if (CoinTossWidgetClass)
    {
        CoinTossWidget = CreateWidget<UCoinTossWidget>(GetWorld(), CoinTossWidgetClass);
        if (CoinTossWidget)
        {
            CoinTossWidget->OnCoinTossCompleted.AddDynamic(this, &AAWGameMode::CoinTossForStartingPlayer);

            CoinTossWidget->AddToViewport();

            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(CoinTossWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            GetWorld()->GetFirstPlayerController()->SetInputMode(InputMode);
            GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;
            bCoinTossActive = true;

        }
    }
}

void AAWGameMode::CoinTossForStartingPlayer(int32 CoinResult)
{
    bCoinTossActive = false;

    StartingPlayer = CoinResult;
    CurrentPlayer = CoinResult;


    if (CoinTossWidget)
    {
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [this]()
            {
                if (CoinTossWidget)
                {
                    CoinTossWidget->RemoveFromParent();
                    CoinTossWidget = nullptr;
                    FInputModeGameOnly GameOnlyMode;
                    GetWorld()->GetFirstPlayerController()->SetInputMode(GameOnlyMode);
                    GetWorld()->GetFirstPlayerController()->FlushPressedKeys();
                    GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;
                }
                PlaceUnitForCurrentPlayer();
            }, 2.0f, false);
    }
    else
    {
        PlaceUnitForCurrentPlayer();
    }
}


void AAWGameMode::PlaceUnitForCurrentPlayer()
{
    if (CurrentPlayer == 1) 
    {
        ATile* RandomTile = GField->GetRandomFreeTile();
        if (RandomTile)
        {
            FVector2D Position = RandomTile->GetGridPosition();
            bool bPlaceSniper = false;

            if (!bSniperPlaced[1])
            {
                bPlaceSniper = true;
            }
            else if (!bBrawlerPlaced[1])
            {
                bPlaceSniper = false;
            }
            else
            {
                bPlaceSniper = FMath::RandBool();
            }

            UWorld* World = GetWorld();
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            FVector SpawnLocation = GField->GetRelativePositionByXYPosition(Position.X, Position.Y);
            SpawnLocation.Z += 5.0f;

            UAWGameInstance* GI = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
            bool bUseAStar = (GI && GI->OpponentType == 1);

            if (bPlaceSniper)
            {
                ASniper* SpawnedUnit = nullptr;
                if (bUseAStar)
                {
                    if (AStarSniperClass)
                    {
                        SpawnedUnit = World->SpawnActor<ASniper>(AStarSniperClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    }
                }
                else
                {
                    if (AISniperClass)
                    {
                        SpawnedUnit = World->SpawnActor<ASniper>(AISniperClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    }
                }

                if (SpawnedUnit)
                {
                    bSniperPlaced.Add(1, true);
                    SpawnedUnit->SetPlayerOwner(1);
                    SpawnedUnit->SetGridPosition(Position.X, Position.Y);
                    //UE_LOG(LogTemp, Log, TEXT("AI has placed a Sniper in %s"), *Position.ToString());

                    if (GField && GField->TileMap.Contains(Position))
                    {
                        ATile* Tile = GField->TileMap[Position];
                        if (Tile)
                        {
                            Tile->SetTileStatus(1, ETileStatus::OCCUPIED, SpawnedUnit);
                        }
                    }
                    int32 NewUnitKey = GField->GameUnitMap.Num();
                    GField->GameUnitMap.Add(NewUnitKey, SpawnedUnit);
                }
            }
            else
            {
                ABrawler* SpawnedUnit = nullptr;
                if (bUseAStar)
                {
                    if (AStarBrawlerClass)
                    {
                        SpawnedUnit = World->SpawnActor<ABrawler>(AStarBrawlerClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    }
                }
                else
                {
                    if (AIBrawlerClass)
                    {
                        SpawnedUnit = World->SpawnActor<ABrawler>(AIBrawlerClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    }
                }

                if (SpawnedUnit)
                {
                    bBrawlerPlaced.Add(1, true);
                    SpawnedUnit->SetPlayerOwner(1);
                    SpawnedUnit->SetGridPosition(Position.X, Position.Y);
                    //UE_LOG(LogTemp, Log, TEXT("AI has placed a Brawler in %s"), *Position.ToString());
                    if (GField && GField->TileMap.Contains(Position))
                    {
                        ATile* Tile = GField->TileMap[Position];
                        if (Tile)
                        {
                            Tile->SetTileStatus(1, ETileStatus::OCCUPIED, SpawnedUnit);
                        }
                    }
                    int32 NewUnitKey = GField->GameUnitMap.Num();
                    GField->GameUnitMap.Add(NewUnitKey, SpawnedUnit);
                }
            }
        }

        NextTurn();
    }
    else 
    {
        
    }

    if (bSniperPlaced[0] && bBrawlerPlaced[0] && bSniperPlaced[1] && bBrawlerPlaced[1])
    {
        CurrentPhase = EGamePhase::Battle;
        //UE_LOG(LogTemp, Log, TEXT("All units placed. Battle phase."));
        NextTurn();
    }
}


int32 AAWGameMode::GetNextPlayer(int32 Player)
{
    Player++;

    if (!Players.IsValidIndex(Player))
    {
        Player = 0;
    }
    return Player;
}


void AAWGameMode::NextTurn()
{
    if (bIsGameOver)
    {
        return;
    }

    if (CurrentPhase == EGamePhase::Placement)
    {
        bool bHumanDone = bSniperPlaced.FindRef(0) && bBrawlerPlaced.FindRef(0);
        bool bAIDone = bSniperPlaced.FindRef(1) && bBrawlerPlaced.FindRef(1);
        if (!(bHumanDone && bAIDone))
        {
            CurrentPlayer = GetNextPlayer(CurrentPlayer);
            SelectedUnit = nullptr;
            PlaceUnitForCurrentPlayer();
            return;
        }


        else
        {
            CurrentPhase = EGamePhase::Battle;
            CurrentPlayer = StartingPlayer;
            bFirstBattleTurn = true;
           
        }
    }

    if (CurrentPhase == EGamePhase::Battle)
    {
        if (bFirstBattleTurn)
        {
            bFirstBattleTurn = false;
        }
        else
        {
            CurrentPlayer = GetNextPlayer(CurrentPlayer);
        }
        ResetActionsForPlayer(CurrentPlayer);

        if (CurrentPlayer == 1)
        {
            UObject* AIObject = Players[1];
            //UE_LOG(LogTemp, Log, TEXT("AI Player class: %s"), *AIObject->GetClass()->GetName());
            AAStarPlayer* AIPlayer = Cast<AAStarPlayer>(AIObject);
            if (AIPlayer)
            {
                if (!bIsAITurnInProgress)
                {
                    bIsAITurnInProgress = true;
                    AIPlayer->OnTurn();
                }
            }
            else
            {
                ARandomPlayer* RandomAI = Cast<ARandomPlayer>(AIObject);
                if (RandomAI)
                {
                    if (!bIsAITurnInProgress)
                    {
                        bIsAITurnInProgress = true;
                        RandomAI->OnTurn();
                    }
                }
            }
        }

        else if (CurrentPlayer == 0)
        {
            bIsAITurnInProgress = false;
            AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(Players[0]);
            if (HumanPlayer)
            {
                
                HumanPlayer->OnTurn();

            }
        }
    }

}

bool AAWGameMode::AllUnitsHaveActed(int32 Player)
{
    for (auto& Pair : GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;

        if (Unit && Unit->GetPlayerOwner() == Player && !(Unit->bHasMoved && Unit->bHasAttacked))
        {
            return false;
        }
    }
    return true;
}

void AAWGameMode::ResetActionsForPlayer(int32 Player)
{
    for (auto& Pair : GField->GameUnitMap)
    {
        AGameUnit* Unit = Pair.Value;
        if (Unit && Unit->GetPlayerOwner() == Player)
        {
            Unit->bHasMoved = false;
            Unit->bHasAttacked = false;
        }
    }
}

void AAWGameMode::EndGame()
{
    bIsGameOver = true;

    bool bHumanUnitsRemaining = false;
    bool bAIUnitsRemaining = false;

    for (auto& UnitPair : GField->GameUnitMap)
    {
        if (UnitPair.Value)
        {
            if (UnitPair.Value->GetPlayerOwner() == 0)
            {
                bHumanUnitsRemaining = true;
            }
            else if (UnitPair.Value->GetPlayerOwner() == 1)
            {
                bAIUnitsRemaining = true;
            }
        }
    }

    for (AActor* PlayerActor : Players)
    {
        UAWGameInstance* GI = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
        if (GI)
        {
            if (bHumanUnitsRemaining && !bAIUnitsRemaining)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1,
                    10.f,
                    FColor::Green,
                    TEXT("HUMAN WINS!"),
                    true,
                    FVector2D(2.f, 2.f) 
                );
            }
            else if (!bHumanUnitsRemaining && bAIUnitsRemaining)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1,           
                    10.f,          
                    FColor::Green,  
                    TEXT("AI WINS!"),
                    true,
                    FVector2D(2.f, 2.f) 
                );
            }
        }
    }
}

