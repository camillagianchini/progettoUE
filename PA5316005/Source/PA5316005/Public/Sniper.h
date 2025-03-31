#pragma once

#include "GameUnit.h"

#include "CoreMinimal.h"
#include "Sniper.generated.h"

UCLASS()
class PA5316005_API ASniper : public AGameUnit
{
	GENERATED_BODY()

public:
   
    ASniper();

    
    UFUNCTION(BlueprintCallable, Category = "Game Unit|Sniper")
    void HandleCounterAttack(AGameUnit* AttackedUnit);

protected:
    
    virtual void BeginPlay() override;

   
    virtual TArray<FVector2D> CalculateAttackMoves() const override;
};




