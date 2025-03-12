#pragma once

#include "AWGameMode.h"
#include "AWGameInstance.h"
#include "PlayerInterface.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HumanPlayer.generated.h"

UCLASS()
class PA5316005_API AHumanPlayer : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:
	// Costruttore
	AHumanPlayer();

protected:
	// Chiamato all’avvio del gioco o quando spawnato
	virtual void BeginPlay() override;
	bool IsMyTurn = false;

public:
	// Chiamato ogni frame
	virtual void Tick(float DeltaTime) override;

	// Camera component attached to player pawn
	UCameraComponent* Camera;

	// Game instance reference
	UAWGameInstance* GameInstance;

	// Game Mode reference
	AAWGameMode* GameMode;

	// IPlayerInterface
	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Metodo chiamato dal PlayerController (AWPlayerController) quando si clicca
	UFUNCTION()
	void OnClick();

	// Se in fase di Turn, gestisce il click su una tile
	void HandleTileClick(class ATile* ClickedTile);

	// Se in Placement, piazza un’unità su una tile
	void HandlePlacementClick(class ATile* ClickedTile);

	// Se in fase di Turn, gestisce il click su un’unità
	void HandleGameUnitClick(class AGameUnit* ClickedUnit);

	// Execute the move
	void ExecuteTheMoveForHumanPlayer(const ATile* EndTile);

	void ExecuteTheAttackForHumanPlayer(const ATile* TargetTile);

};



