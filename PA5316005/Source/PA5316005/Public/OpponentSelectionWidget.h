// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AWGameInstance.h"
#include "OpponentSelectionWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOpponentSelected, int32, SelectedOpponent);

class Button;
/**
 * 
 */
UCLASS()
class PA5316005_API UOpponentSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    // Bottoni da bindare
    UPROPERTY(meta = (BindWidget))
    class UButton* RandomButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* AStarButton;

    // Dispatcher per notificare la scelta
    UPROPERTY(BlueprintAssignable, Category = "Opponent")
    FOnOpponentSelected OnOpponentSelected;

protected:
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void HandleRandomClicked();

    UFUNCTION()
    void HandleAStarClicked();
	
};
