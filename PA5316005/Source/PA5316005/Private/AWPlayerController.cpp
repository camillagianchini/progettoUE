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

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(AWContext, 0);

        if (AWContext)
        {
            UE_LOG(LogTemp, Log, TEXT("Added Mapping Context: %s"), *AWContext->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AWContext è nullptr."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Subsystem not found!"));
    }
}


void AAWPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!ClickAction)
    {
        UE_LOG(LogTemp, Warning, TEXT("ClickAction è nullptr!"));
    }

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AAWPlayerController::ClickOnGrid);
        UE_LOG(LogTemp, Log, TEXT("BindAction per ClickAction effettuato con successo."));
    }
}


void AAWPlayerController::ClickOnGrid()
{
    UE_LOG(LogTemp, Warning, TEXT("ClickOnGrid() triggered!"));
	// Se il Pawn controllato è un HumanPlayer, chiamalo per gestire il click
	if (AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(GetPawn()))
	{
		HumanPlayer->OnClick();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Il Pawn posseduto non è un HumanPlayer."));
	}
}

