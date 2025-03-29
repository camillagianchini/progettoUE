#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AWGameInstance.generated.h"

UCLASS()
class PA5316005_API UAWGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:


	// Messaggio da mostrare ad ogni turno (es. "Human Turn", "AI Turn")
	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	int32 OpponentType = 0;

	// ************ SETTERS ************
	// Imposta il messaggio del turno
	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message);

	UFUNCTION(BlueprintCallable)
	void SetOpponent(const int32 SelectedOpp);


	// Ritorna il messaggio corrente del turno
	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage();


};

