#include "Obstacles.h"
#include "Components/StaticMeshComponent.h"

// Costruttore
AObstacles::AObstacles()
{
    PrimaryActorTick.bCanEverTick = false;

    // Creazione del componente mesh
    ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
    RootComponent = ObstacleMesh;
}

// Chiamato all'inizio del gioco
void AObstacles::BeginPlay()
{
    Super::BeginPlay();

    // Imposta il materiale in base al tipo di ostacolo
    switch (ObstacleType)
    {
    case EObstacleType::Mountain:
        ObstacleMesh->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Mountain")));
        break;
    case EObstacleType::Tree:
        ObstacleMesh->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Tree")));
        break;
    }
}

// Imposta il tipo di ostacolo
void AObstacles::SetObstacleType(EObstacleType Type)
{
    ObstacleType = Type;
}

// Imposta la posizione sulla griglia
void AObstacles::SetGridPosition(FVector2D NewPosition)
{
    GridPosition = NewPosition;
}

