#pragma once

#include "CoreMinimal.h"
#include "GameUnit.h"
#include "Sniper.generated.h"

UCLASS()
class PA5316005_API ASniper : public AGameUnit
{
	GENERATED_BODY()

public:
	// ************ CONSTRUCTORS ************
	ASniper();

	// ************ METHODS ************
	// Gestisce il contrattacco: se le condizioni sono verificate, lo sniper subisce un danno da contrattacco.
	UFUNCTION(BlueprintCallable, Category = "Game Unit|Sniper")
	void HandleCounterAttack(AGameUnit* AttackedUnit);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};




