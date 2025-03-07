#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Sniper.h"   // Include la classe Sniper
#include "Brawler.h"  // Include la classe Brawler
#include "Obstacles.h"  // Include la classe per gli ostacoli
#include "MovesPanel.h"
#include "AWGameInstance.generated.h"

UCLASS()
class PA5316005_API UAWGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;

    /** Mappa delle unità sulla griglia */
    UPROPERTY(BlueprintReadOnly)
    TMap<FVector2D, ASniper*> SniperGridMap;

    UPROPERTY(BlueprintReadOnly)
    TMap<FVector2D, ABrawler*> BrawlerGridMap;

    /** Mappa degli ostacoli */
    UPROPERTY(BlueprintReadOnly)
    TMap<FVector2D, AObstacles*> ObstacleMap;

    /** Storico delle mosse */
    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Moves;

    /** Pannello UI delle mosse */
    UPROPERTY(BlueprintReadOnly)
    UMovesPanel* MovesPanel;

    /** Giocatore corrente (Human o AI) */
    UPROPERTY(BlueprintReadOnly)
    bool bIsHumanTurn;  // True se il giocatore è in turno

    /** Funzioni principali */
    UFUNCTION(BlueprintCallable)
    void InitGame();

    UFUNCTION(BlueprintCallable)
    void AddMove(const FString& MoveString);

    void GenerateObstacles(float Percentage);  // Genera ostacoli con percentuale
    void SwitchTurn();  // Gestisce il cambio del turno

    /** Aggiungi unità alla griglia */
    void AddUnitToGrid(FVector2D Position, bool bIsSniper);

    // Gestisce la logica di attacco per Sniper
    UFUNCTION(BlueprintCallable)
    void HandleSniperAttack(ASniper* Attacker, AUnit* Defender);

    // Gestisce la logica di attacco per Brawler
    UFUNCTION(BlueprintCallable)
    void HandleBrawlerAttack(ABrawler* Attacker, AUnit* Defender);

};
