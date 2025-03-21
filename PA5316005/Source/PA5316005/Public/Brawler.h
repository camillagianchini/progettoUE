#pragma once

#include "GameUnit.h"

#include "CoreMinimal.h"
#include "Brawler.generated.h"

UCLASS()
class PA5316005_API ABrawler : public AGameUnit
{
	GENERATED_BODY()

public:
	// ************ CONSTRUCTORS ************
	ABrawler();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual TArray<FVector2D> CalculateAttackMoves() const override;

	// Eventuali metodi specifici per il Brawler possono essere aggiunti qui
};



