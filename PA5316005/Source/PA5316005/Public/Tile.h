#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UENUM(BlueprintType)
enum class ETileStatus : uint8
{
	EMPTY       UMETA(DisplayName = "Empty"),
	OCCUPIED    UMETA(DisplayName = "Occupied"),
	OBSTACLE    UMETA(DisplayName = "Obstacle"),
};

UCLASS()
class PA5316005_API ATile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATile();

	// Sets the owner and the status of a tile.
	// Se si imposta lo stato OBSTACLE, l'owner rimane -1.
	void SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus);

	// Returns the current tile status.
	ETileStatus GetTileStatus() const;

	// Returns the current owner (-1 se nessuno o se è un ostacolo).
	int32 GetOwner() const;

	// Sets the (x, y) grid position of the tile.
	void SetGridPosition(const double InX, const double InY);

	// Returns the (x, y) grid position of the tile.
	FVector2D GetGridPosition() const;

	// Returns true if the tile is walkable (i.e. non occupata e non un ostacolo).
	bool IsWalkable() const;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	// Componente di scena
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	// Componente mesh per la visualizzazione
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	// Stato attuale della tile: EMPTY, OCCUPIED oppure OBSTACLE.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETileStatus Status;

	// Identificativo del proprietario della tile (-1 se nessuno).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerOwner;

	// Posizione (x, y) della tile nella griglia.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TileGridPosition;
};
