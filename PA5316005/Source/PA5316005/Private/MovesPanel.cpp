#include "MovesPanel.h"
#include "MoveHistoryWidget.h"
#include "Components/ScrollBox.h"

UScrollBox* UMovesPanel::GetScrollBox() const
{
	return ScrollBox;
}

void UMovesPanel::AddMoveToPanel(const FGameMove& Move)
{
    if (!ScrollBox || !MoveHistoryWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("MovesPanel: ScrollBox o MoveWidgetEntryClass non assegnati!"));
        return;
    }

    // Siccome "this" è un UUserWidget, spesso si fa CreateWidget<>(GetWorld(), ...)
    UMoveHistoryWidget* MoveEntry = CreateWidget<UMoveHistoryWidget>(GetWorld(), MoveHistoryWidgetClass);
    if (MoveEntry)
    {
        MoveEntry->SetData(Move);
        ScrollBox->AddChild(MoveEntry);
        ScrollBox->ScrollToEnd();
    }
}

void UMovesPanel::PopMoveFromPanel()
{
	if (ScrollBox && ScrollBox->GetChildrenCount() > 0)
	{
		// Rimuovi l'ultimo widget
		UWidget* LastChild = ScrollBox->GetChildAt(ScrollBox->GetChildrenCount() - 1);
		if (LastChild)
		{
			LastChild->RemoveFromParent();
		}
	}
}
