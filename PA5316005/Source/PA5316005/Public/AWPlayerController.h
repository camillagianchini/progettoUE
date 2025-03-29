#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AWPlayerController.generated.h"

UCLASS()
class PA5316005_API AAWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AAWPlayerController();



	// Input mapping context per il gioco a turni
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* AWContext;

	// L'azione di click (da definire nell'Enhanced Input)
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* ClickAction;

	// Metodo chiamato al click del mouse; viene passato al Pawn (HumanPlayer)
	void ClickOnGrid();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;



};

