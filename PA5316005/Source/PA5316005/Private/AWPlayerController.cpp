#include "AWPlayerController.h"
#include "HumanPlayer.h"  // Assicurati che il percorso corrisponda
#include "Components/InputComponent.h"

AAWPlayerController::AAWPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
}

void AAWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Se utilizzi Enhanced Input, aggiungi il mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(AWContext, 0);
	}
}

void AAWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind dell'azione click usando Enhanced Input
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AAWPlayerController::ClickOnGrid);
	}
}

void AAWPlayerController::ClickOnGrid()
{
	// Se il Pawn controllato è un HumanPlayer, chiamalo per gestire il click
	if (AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(GetPawn()))
	{
		HumanPlayer->OnClick();
	}
}

