#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AWGameInstance.generated.h"

UCLASS()
class PA5316005_API UAWGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// ************ ATTRIBUTES ************
	// Punteggio per il giocatore umano
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	int32 ScoreHumanPlayer = 0;

	// Punteggio per il giocatore AI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	int32 ScoreAiPlayer = 0;

	// Messaggio da mostrare ad ogni turno (es. "Human Turn", "AI Turn")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	FString CurrentTurnMessage = TEXT("Current Player");

	// ************ SETTERS ************
	// Imposta il messaggio del turno
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetTurnMessage(const FString& Message);

	// Imposta il tipo di avversario o la difficoltà (puoi usarlo per selezionare, ad esempio, AI Random o AStar)
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetOpponent(const int32 SelectedOpp);

	// ************ GETTERS ************
	// Ritorna il punteggio del giocatore umano
	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetScoreHumanPlayer() const;

	// Ritorna il punteggio del giocatore AI
	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetScoreAiPlayer() const;

	// Ritorna il messaggio corrente del turno
	UFUNCTION(BlueprintCallable, Category = "Game")
	FString GetTurnMessage() const;

	// ************ METHODS ************
	// Incrementa il punteggio del giocatore umano
	UFUNCTION(BlueprintCallable, Category = "Game")
	void IncrementScoreHumanPlayer();

	// Incrementa il punteggio del giocatore AI
	UFUNCTION(BlueprintCallable, Category = "Game")
	void IncrementScoreAiPlayer();
};

