#pragma once

#include "CoreMinimal.h"
#include "GameUnit.h"
#include "Sniper.generated.h"

UCLASS()
class PA5316005_API ASniper : public AGameUnit
{
	GENERATED_BODY()

public:
	ASniper();

	// Override di AttackUnit per gestire il contrattacco
	virtual void AttackUnit(AGameUnit* TargetUnit) override;

protected:
	// Danno random in caso di contrattacco
	int32 GetCounterAttackDamage() const;
};



