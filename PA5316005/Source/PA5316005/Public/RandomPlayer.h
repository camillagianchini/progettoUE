// RandomPlayer.h

#pragma once

#include "CoreMinimal.h"
#include "PlayerInterface.h"
#include "RandomPlayer.generated.h"

UCLASS()
class PA5316005_API ARandomPlayer : public AActor, public IPlayerInterface
{
    GENERATED_BODY()

public:
    virtual void OnTurn() override;

    ARandomPlayer();

private:
    void PerformRandomActionOnUnit(class AGameUnit* Unit);
};


