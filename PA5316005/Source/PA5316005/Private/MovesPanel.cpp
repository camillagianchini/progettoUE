#include "MovesPanel.h"
#include "Components/ScrollBox.h"
#include "MoveWidgetEntry.h"

void UMovesPanel::AddMove(const FString& MoveString)
{
    if (MoveEntryClass && MovesList)
    {
        UMoveWidgetEntry* NewEntry = CreateWidget<UMoveWidgetEntry>(GetWorld(), MoveEntryClass);
        if (NewEntry)
        {
            NewEntry->SetMoveText(MoveString);
            MovesList->AddChild(NewEntry);
        }
    }
}
