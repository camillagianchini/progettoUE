#pragma once

#include "GameUnit.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UENUM(BlueprintType)
enum class ETileStatus : uint8
{
	EMPTY       UMETA(DisplayName = "Empty"),
	OCCUPIED    UMETA(DisplayName = "Occupied"),
	OBSTACLE    UMETA(DisplayName = "Obstacle")
};

UENUM()
enum class ETileGameStatus : uint8
{
	FREE UMETA(DisplayName = "Free"),
	SELECTED UMETA(DisplayName = "Selected"),
	LEGAL_MOVE UMETA(DisplayName = "Legal Move"),
	CAN_ATTACK UMETA(DisplayName = "Can Attack"),
};

UCLASS()
class PA5316005_API ATile : public AActor
{
	GENERATED_BODY()

public:
   
    ATile();

   
    virtual void BeginPlay() override;

    // --- Methods ---

   
    void SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus, AGameUnit* TileGameUnit);

    void SetTileGameStatus(ETileGameStatus NewTileGameStatus);

    
    void SetGridPosition(const double InX, const double InY);

   
    void SetTileMaterial() const;

    // --- Getters ---
    ETileStatus GetTileStatus() const;
    ETileGameStatus GetTileGameStatus() const;
    inline bool IsLegalTile() const;
    FVector2D GetGridPosition() const;
    int32 GetTileOwner() const;
    AGameUnit* GetGameUnit() const;
    FString GameStatusToString() const;

public:
   
    UPROPERTY(VisibleAnywhere)
    class UTextRenderComponent* TileTextNumber;

    UPROPERTY(VisibleAnywhere)
    class UTextRenderComponent* TileTextLetter;

protected:
    // --- Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Scene;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StaticMeshComponent;

    // --- Properties ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Property")
    ETileStatus Status;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Property")
    ETileGameStatus TileGameStatus;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Property")
    bool bIsLegal;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Property")
    FVector2D TileGridPosition;

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Property")
    int32 PlayerOwner;

   
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Property")
    AGameUnit* GameUnit;

    // --- Materials for Obstacle  ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacles")
    UMaterialInterface* ObstacleMaterial1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacles")
    UMaterialInterface* ObstacleMaterial2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacles")
    UMaterialInterface* ObstacleMaterial3;
};
