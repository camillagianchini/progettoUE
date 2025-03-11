#pragma once

#include "CoreMinimal.h"
#include "GameUnit.h"
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

	// Eventuali metodi specifici per il Brawler possono essere aggiunti qui
};



