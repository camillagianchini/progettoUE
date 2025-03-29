#include "AWPlayerController.h"
#include "HumanPlayer.h"  // Assicurati che il percorso corrisponda
#include "CoinTossWidget.h"

#include "Components/InputComponent.h"


class AWGameMode;

AAWPlayerController::AAWPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;

}

void AAWPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(AWContext, 0);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Subsystem not found!"));
    }

}



void AAWPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();


    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AAWPlayerController::ClickOnGrid);
    }
}



void AAWPlayerController::ClickOnGrid()
{
    UE_LOG(LogTemp, Log, TEXT("ClickOnGrid chiamato."));

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (GM->bCoinTossActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ignoro il click, coin toss attivo."));
        return;
    }

    const auto HumanPlayer = Cast<AHumanPlayer>(GetPawn());
    if (IsValid(HumanPlayer))
    {
        UE_LOG(LogTemp, Log, TEXT("Invoco OnClick sul HumanPlayer."));
        HumanPlayer->OnClick();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ClickOnGrid: HumanPlayer non valido."));
    }
}



