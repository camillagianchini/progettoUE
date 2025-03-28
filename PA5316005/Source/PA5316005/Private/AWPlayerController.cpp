#include "AWPlayerController.h"
#include "HumanPlayer.h"  // Assicurati che il percorso corrisponda
#include "CoinTossWidget.h"

#include "Components/InputComponent.h"
#include "InputMappingContext.h"

class AWGameMode;

AAWPlayerController::AAWPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;

    static ConstructorHelpers::FObjectFinder<UInputMappingContext> ContextFinder(
        TEXT("/Game/Input/IMC_AW.IMC_AW") // <-- Percorso all’asset
    );
    if (ContextFinder.Succeeded())
    {
        AWContext = ContextFinder.Object;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Impossibile trovare l'asset IMC_AW!"));
    }
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
        //UE_LOG(LogTemp, Log, TEXT("BindAction completato per ClickAction."));
    }
}



void AAWPlayerController::ClickOnGrid()
{
    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (GM->bCoinTossActive)  // oppure un check su un widget, su un bool, ecc.
    {
        UE_LOG(LogTemp, Warning, TEXT("Ignoro il click, coin toss attivo"));
        return;
    }

    const auto HumanPlayer = Cast<AHumanPlayer>(GetPawn());
    if (IsValid(HumanPlayer))
    {
        HumanPlayer->OnClick();
    }

}


