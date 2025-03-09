#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AWPlayerController.generated.h"

UCLASS()
class PA5316005_API AAWPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AAWPlayerController();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Enhanced")
    UInputMappingContext* AWMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Enhanced")
    UInputAction* ClickAction;

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    AActor* SelectedUnit;  // L'unità attualmente selezionata
    bool bHasMoved = false;  // Controlla se l'unità ha già mosso
    bool bHasAttacked = false;  // Controlla se l'unità ha già attaccato

    UFUNCTION()
    void OnClick(const FInputActionValue& Value);

    void HandleClick();  // Logica principale del click
    void SelectUnit(AActor* NewSelectedUnit);  // Seleziona Sniper o Brawler
    void MoveSelectedUnit(FVector2D TargetPosition);  // Movimento
    void AttackWithSelectedUnit(AActor* Target);  // Attacco
    bool IsMoveValid(FVector2D Start, FVector2D Target, int32 MovementRange);  // Controllo range
    void ShowMovementRange();  // Evidenzia le celle in cui può muoversi
    void ClearMovementRange(); // Rimuove l'evidenziazione
    void ShowAttackRange();
    void ClearAttackRange();
    void EndTurn();
    bool HaveAllUnitsActed();

};

