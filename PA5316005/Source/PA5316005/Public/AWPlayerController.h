#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AWPlayerController.generated.h"

UCLASS()
class PA5316005_API AAWPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AAWPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;  // Dichiarazione corretta

public:
    void HandleClick();  // Funzione per gestire il click sulle unità o sulla griglia
    void SelectUnit(AActor* SelectedUnit);  // Seleziona un'unità
    void MoveSelectedUnit(FVector2D TargetPosition);  // Muove l'unità selezionata
    void AttackWithSelectedUnit(AActor* Target);  // Attacca un'unità nemica
    bool IsMoveValid(FVector2D Start, FVector2D Target, int32 MovementRange); // Controlla se il movimento è valido

    TArray<FVector2D> FindPathAStar(FVector2D Start, FVector2D Target); // Implementazione A*

private:
    AActor* SelectedUnit;  // L'unità attualmente selezionata
};
