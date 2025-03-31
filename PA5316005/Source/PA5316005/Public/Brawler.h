#pragma once

#include "CoreMinimal.h"
#include "GameUnit.h"
#include "Brawler.generated.h"

UCLASS()
class PA5316005_API ABrawler : public AGameUnit
{
	GENERATED_BODY()

public:

	ABrawler();

protected:

	virtual void BeginPlay() override;

	virtual TArray<FVector2D> CalculateAttackMoves() const override;

};



