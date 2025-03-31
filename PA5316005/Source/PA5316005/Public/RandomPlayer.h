#pragma once

#include "CoreMinimal.h"
#include "GameUnit.h"
#include "PlayerInterface.h"
#include "RandomPlayer.generated.h"

UCLASS()
class PA5316005_API ARandomPlayer : public AActor, public IPlayerInterface
{
    GENERATED_BODY()

public:

    virtual void OnTurn() override;

    ARandomPlayer();

    
    UPROPERTY()
    TArray<AGameUnit*> UnitsSequence; 


    int32 SequenceIndex;


private:

    void PerformRandomActionOnUnit(class AGameUnit* Unit);

    void DoNextUnitAction();
};

