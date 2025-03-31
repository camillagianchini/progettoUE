#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AWGameInstance.generated.h"

UCLASS()
class PA5316005_API UAWGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	int32 OpponentType = 0;


	// Setters
	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message);

	UFUNCTION(BlueprintCallable)
	void SetOpponent(const int32 SelectedOpp);


	// Getters
	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage();


};

