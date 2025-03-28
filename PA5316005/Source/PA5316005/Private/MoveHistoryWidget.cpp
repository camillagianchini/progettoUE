// Fill out your copyright notice in the Description page of Project Settings.



#include "MoveHistoryWidget.h"


void UMoveHistoryWidget::SetData(const FGameMove& Move)
{
	// Costruisci la stringa in base al tipo di mossa
	FString StringMove;

	// Formato base: "Player: Unit" (es. "HP: S")
	StringMove = Move.PlayerID + TEXT(" :  ") + Move.UnitType + TEXT(" ");

	if (!Move.bIsAttack)
	{
		// Mossa di movimento: "From -> To"
		StringMove += Move.FromCell + TEXT("->") + Move.ToCell;
	}
	else
	{
		// Mossa di attacco: "Target Danno"
		StringMove += Move.TargetCell + TEXT("  ") + FString::FromInt(Move.Damage);
	}

	// Imposta il testo nel widget
	if (MoveText)
	{
		MoveText->SetText(FText::FromString(StringMove));
	}


	
}

