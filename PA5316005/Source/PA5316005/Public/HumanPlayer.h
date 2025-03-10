#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "Camera/CameraComponent.h"
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

public:
	// Chiamato ogni frame
	virtual void Tick(float DeltaTime) override;

	// IPlayerInterface
	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;
	virtual void MakeMove() override;

	// Metodo chiamato dal PlayerController (AWPlayerController) quando si clicca
	UFUNCTION()
	void OnClick();

	// Se in fase di Turn, gestisce il click su una tile
	void HandleTileClick(class ATile* ClickedTile);

	// Se in fase di Turn, gestisce il click su un’unità
	void HandleUnitClick(class AGameUnit* ClickedUnit);

	// Se in Placement, piazza un’unità su una tile
	void HandlePlacementClick(class ATile* ClickedTile);

	//-----------------------------------------
	// Riferimenti e variabili
	//-----------------------------------------

	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	// Riferimento al GameMode (AWGameMode)
	UPROPERTY()
	class AAWGameMode* GameModeRef;

	// Riferimento al GameInstance (AWGameInstance)
	UPROPERTY()
	class UAWGameInstance* GameInstanceRef;

	// Flag che indica se è il turno del giocatore umano
	bool bIsMyTurn;

	// Unità attualmente selezionata in fase di Turn
	UPROPERTY()
	class AGameUnit* SelectedUnit;

	// (Opzionale) Decide quale unità piazzare in fase di placement: "Sniper" o "Brawler"
	// In un progetto reale potresti farlo scegliere via UI.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	FName UnitToPlace;
};



