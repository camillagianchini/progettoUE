#include "AWPlayerController.h"
#include "HumanPlayer.h"
#include "CoinTossWidget.h"
#include "MovesPanel.h"
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
        //UE_LOG(LogTemp, Warning, TEXT("Subsystem not found!"));
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

    AAWGameMode* GM = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (GM->bCoinTossActive)
    {
        return;
    }

    const auto HumanPlayer = Cast<AHumanPlayer>(GetPawn());
    if (IsValid(HumanPlayer))
    {
        HumanPlayer->OnClick();
    }
    else
    {

    }
}



