#include "MoveWidgetEntry.h"
#include "Components/TextBlock.h"

void UMoveWidgetEntry::SetMoveText(const FString& MoveString)
{
    if (MoveText)
    {
        MoveText->SetText(FText::FromString(MoveString));
    }
}
