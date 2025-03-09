#pragma once

#include "CoreMinimal.h"
#include "RandomPlayer.h"  // Estendiamo RandomPlayer
#include "AStarPlayer.generated.h"

UCLASS()
class PA5316005_API AAStarPlayer : public ARandomPlayer
{
    GENERATED_BODY()

public:
    AAStarPlayer();

    virtual void OnTurn() override;  // Override per usare A*

    void MoveAndAttack(ASniper* Sniper) override;  // Usa A*
    void MoveAndAttack(ABrawler* Brawler) override;  // Usa A*
};

