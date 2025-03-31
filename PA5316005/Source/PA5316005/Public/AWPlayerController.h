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

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* AWContext;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* ClickAction;

	void ClickOnGrid();

protected:

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

};

