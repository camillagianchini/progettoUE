// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "MovesPanel.generated.h"

UCLASS(Abstract)
class PA5316005_API UMovesPanel : public UUserWidget
{
	GENERATED_BODY()

protected:
	// Lista con barra di scorrimento per lo storico delle mosse
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moves")
	TSubclassOf<class UMoveHistoryWidget> MoveHistoryWidgetClass;




public:
	// Restituisce il riferimento allo ScrollBox
	UFUNCTION(BlueprintCallable)
	UScrollBox* GetScrollBox() const;

	// Aggiunge una mossa allo storico (passa la struttura FGameMove)
	UFUNCTION(BlueprintCallable)
	void AddMoveToPanel(const FGameMove& Move);

	// (Opzionale) Rimuove l’ultima mossa dallo storico
	UFUNCTION(BlueprintCallable)
	void PopMoveFromPanel();
};

