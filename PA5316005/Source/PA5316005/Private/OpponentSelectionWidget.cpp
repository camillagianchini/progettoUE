#include "OpponentSelectionWidget.h"
#include "Components/Button.h"

void UOpponentSelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RandomButton)
    {
        RandomButton->OnClicked.AddDynamic(this, &UOpponentSelectionWidget::HandleRandomClicked);
    }
    if (AStarButton)
    {
        AStarButton->OnClicked.AddDynamic(this, &UOpponentSelectionWidget::HandleAStarClicked);
    }
}

void UOpponentSelectionWidget::HandleRandomClicked()
{
    OnOpponentSelected.Broadcast(0); 
    RemoveFromParent();
}

void UOpponentSelectionWidget::HandleAStarClicked()
{
    OnOpponentSelected.Broadcast(1); 
    RemoveFromParent();
}
