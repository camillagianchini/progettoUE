#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"


UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class PA5316005_API IPlayerInterface
{
	GENERATED_BODY()


public:

	int32 PlayerNumber;

	virtual void OnTurn() {};

};