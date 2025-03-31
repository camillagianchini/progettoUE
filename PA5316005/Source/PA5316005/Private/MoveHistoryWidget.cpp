#include "MoveHistoryWidget.h"


void UMoveHistoryWidget::SetData(const FGameMove& Move)
{
	FString StringMove;

	StringMove = Move.PlayerID + TEXT(" :  ") + Move.UnitType + TEXT(" ");

	if (!Move.bIsAttack)
	{
		StringMove += Move.FromCell + TEXT("->") + Move.ToCell;
	}
	else
	{
		StringMove += Move.TargetCell + TEXT("  ") + FString::FromInt(Move.Damage);
	}

	if (MoveText)
	{
		MoveText->SetText(FText::FromString(StringMove));
	}
}

