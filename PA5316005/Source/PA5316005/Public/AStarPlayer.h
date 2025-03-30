// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameUnit.h"
#include "PlayerInterface.h"
#include "AStarPlayer.generated.h"

UCLASS()
class PA5316005_API AAStarPlayer : public AActor, public IPlayerInterface
{
	GENERATED_BODY()
	
public:
	AAStarPlayer();

	// Metodo che viene chiamato quando è il turno di questo player
	virtual void OnTurn() override;

private:
	TArray<AGameUnit*> UnitsSequence;
	int32 SequenceIndex;

	TArray<FVector2D> AStarPathfinding(class AGameUnit* Unit, const FVector2D& GoalPos);

	// Restituisce la cella target, ovvero la cella lungo il percorso raggiungibile entro il range di movimento
	FVector2D GetTargetPositionForUnit(class AGameUnit* Unit, const FVector2D& EnemyPos);

	// Trova la posizione dell'unità nemica più vicina
	FVector2D GetClosestEnemyPosition(class AGameUnit* Unit);

	// Esegue l'azione per l'unità: spostamento verso il nemico più vicino (nel limite del movimento)
	void PerformAStarActionOnUnit(class AGameUnit* Unit);

	// Funzione che gestisce le azioni delle unità in sequenza (sostituisce la logica casuale)
	virtual void DoNextUnitAction();

	bool bTurnEnded;
};
