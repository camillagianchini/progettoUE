#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Obstacles.generated.h"

// Enum per identificare il tipo di ostacolo
UENUM(BlueprintType)
enum class EObstacleType : uint8
{
    Mountain  UMETA(DisplayName = "Mountain"),
    Tree      UMETA(DisplayName = "Tree")
};

UCLASS()
class PA5316005_API AObstacles : public AActor
{
    GENERATED_BODY()

public:
    // Costruttore
    AObstacles();

protected:
    virtual void BeginPlay() override;

public:
    // Mesh dell'ostacolo
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
    class UStaticMeshComponent* ObstacleMesh;

    // Tipo di ostacolo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Settings")
    EObstacleType ObstacleType;

    // Posizione sulla griglia
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
    FVector2D GridPosition;

    // Imposta il tipo di ostacolo
    void SetObstacleType(EObstacleType Type);

    // Imposta la posizione dell'ostacolo
    void SetGridPosition(FVector2D NewPosition);
};

