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

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moves")
	TSubclassOf<class UMoveHistoryWidget> MoveHistoryWidgetClass;

public:
	
	UFUNCTION(BlueprintCallable)
	UScrollBox* GetScrollBox() const;

	UFUNCTION(BlueprintCallable)
	void AddMoveToPanel(const FGameMove& Move);

	UFUNCTION(BlueprintCallable)
	void PopMoveFromPanel();
};

