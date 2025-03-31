
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

	virtual void OnTurn() override;

private:
	TArray<AGameUnit*> UnitsSequence;
	int32 SequenceIndex;

	TArray<FVector2D> AStarPathfinding(class AGameUnit* Unit, const FVector2D& GoalPos);

	FVector2D GetTargetPositionForUnit(class AGameUnit* Unit, const FVector2D& EnemyPos);

	FVector2D GetClosestEnemyPosition(class AGameUnit* Unit);

	void PerformAStarActionOnUnit(class AGameUnit* Unit);

	virtual void DoNextUnitAction();

	bool bTurnEnded;
};
