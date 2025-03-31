#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AWGameMode.h"
#include "AWGameInstance.h"
#include "PlayerInterface.h"
#include "HumanPlayer.generated.h"

UCLASS()
class PA5316005_API AHumanPlayer : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:

	AHumanPlayer();

protected:

	virtual void BeginPlay() override;

	bool IsMyTurn = false;

public:
	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	UAWGameInstance* GameInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	AAWGameMode* GameMode;
	
	virtual void OnTurn() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void OnClick();

	UFUNCTION(BlueprintCallable, Category = "Player")
	FVector2D GetClickedTilePosition() const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void DoNextUnitAction();
};