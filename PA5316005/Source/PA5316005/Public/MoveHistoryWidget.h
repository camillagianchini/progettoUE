#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "AWGameMode.h"
#include "MoveHistoryWidget.generated.h"


UCLASS()
class PA5316005_API UMoveHistoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HorizontalBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MoveText;

	UFUNCTION(BlueprintCallable)
	void SetData(const FGameMove& Move);
};
