#include "AWPlayerController.h"
#include "HumanPlayer.h"  // Assicurati che il percorso corrisponda
#include "Components/InputComponent.h"
#include "InputMappingContext.h"


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
        UE_LOG(LogTemp, Log, TEXT("Added Mapping Context: %s"), AWContext ? *AWContext->GetName() : TEXT("None"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Subsystem not found!"));
    }

    // Imposta l'Input Mode per il gioco
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
}



void AAWPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!ClickAction)
    {
        UE_LOG(LogTemp, Warning, TEXT("ClickAction è nullptr! Assicurati di assegnarlo nell'editor."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("ClickAction assegnato: %s"), *ClickAction->GetName());
    }

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AAWPlayerController::ClickOnGrid);
        UE_LOG(LogTemp, Log, TEXT("BindAction completato per ClickAction."));
    }
}



void AAWPlayerController::ClickOnGrid()
{
    UE_LOG(LogTemp, Warning, TEXT("ClickOnGrid() triggered!"));
    if (AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(GetPawn()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Trovato HumanPlayer!"));
        HumanPlayer->OnClick();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Il Pawn posseduto non è un HumanPlayer."));
    }
}


