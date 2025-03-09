#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "Sniper.h"
#include "Brawler.h"
#include "AWGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "RandomPlayer.generated.h"

UCLASS()
class PA5316005_API ARandomPlayer : public APawn, public IPlayerInterface
{
    GENERATED_BODY()

public:
    ARandomPlayer();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Game Mode reference
    AAWGameMode* GameMode;

    // Funzioni IA
    virtual void OnTurn() override;
    virtual void OnWin() override;
    virtual void OnLose() override;

    void DecideMove();          // Decide la prossima mossa
    void SelectRandomUnit();    // Seleziona casualmente Sniper o Brawler
    void MoveAndAttack(ASniper* Sniper);  // Logica per Sniper
    void MoveAndAttack(ABrawler* Brawler); // Logica per Brawler
};

