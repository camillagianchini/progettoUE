#include "AWGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

void UAWGameInstance::Init()
{
    Super::Init();
    StartGame();
}

void UAWGameInstance::StartGame()
{
    // **Coin Toss** per determinare chi inizia
    int32 CoinFlip = FMath::RandRange(0, 1);
    bIsHumanTurn = (CoinFlip == 0); // 0 = Human, 1 = AI

    // Stampa nel log chi ha vinto il lancio della moneta
    UE_LOG(LogTemp, Warning, TEXT("Coin Toss Result: %s starts"), bIsHumanTurn ? TEXT("Human") : TEXT("AI"));

    // Inizializza il pannello della UI se esiste
    if (MovesPanelClass)
    {
        MovesPanelInstance = CreateWidget<UMovesPanel>(GetWorld(), MovesPanelClass);
        if (MovesPanelInstance)
        {
            MovesPanelInstance->AddToViewport();
        }
    }
}

void UAWGameInstance::ToggleTurn()
{
    bIsHumanTurn = !bIsHumanTurn;
    UE_LOG(LogTemp, Warning, TEXT("Turno cambiato, ora gioca: %s"), bIsHumanTurn ? TEXT("Human") : TEXT("AI"));
}

void UAWGameInstance::AddMoveToHistory(const FString& MoveString)
{
    MoveHistory.Add(MoveString);
    UE_LOG(LogTemp, Warning, TEXT("Mossa registrata: %s"), *MoveString);

    // Aggiorna la UI delle mosse
    if (MovesPanelInstance)
    {
        MovesPanelInstance->AddMove(MoveString);
    }
}
