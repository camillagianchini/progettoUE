#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameUnit.generated.h"

class AAWGameMode;
class AGameField;

UENUM()
enum class EGameUnitType : uint8
{
	SNIPER UMETA(DisplayName = "Sniper"),
	BRAWLER UMETA(DisplayName = "Brawler")
};

UCLASS()
class PA5316005_API AGameUnit : public AActor
{
	GENERATED_BODY()

public:
	// ************ CONSTRUCTORS ************
	// Sets default values for this actor's properties
	AGameUnit();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ************ ATTRIBUTES ************
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	// To track the last used ID
	static int32 NewGameUnitID;

	// ID of the Game Unit
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 GameUnitID;

	// Player Owner
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerOwner;

	// Game Unit's position in the grid
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D GameUnitGridPosition;

	// Type indicates (SNIPER or BRAWLER)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGameUnitType GameUnitType;

	


	// Health Points of the unit
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 HitPoints;

	// Movement range (number of cells the unit can move)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MovementRange;

	// Attack range of the unit
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 AttackRange;

	// Minimum damage the unit can inflict
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 DamageMin;

	// Maximum damage the unit can inflict
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 DamageMax;

	// Reference to GameMode
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AAWGameMode* GameMode;



public:
	// ************ SETTERS ************
	// Set a unique GameUnit ID (e.g., auto-increment)
	void SetGameUnitID();

	// Set the Player Owner of the unit
	void SetPlayerOwner(int32 NewPlayerOwner);

	// Set the (x, y) grid position
	void SetGridPosition(const double InX, const double InY);

	// Set the type of the Game Unit (SNIPER or BRAWLER)
	void SetGameUnitType(EGameUnitType NewType);

	// Set the unit's Hit Points
	void SetHitPoints(int32 NewHitPoints);

	// Set the unit's Movement Range
	void SetMovementRange(int32 NewRange);

	// Set the unit's Attack Range
	void SetAttackRange(int32 NewAttackRange);

	// Set the unit's Damage values (min and max)
	void SetDamage(int32 NewDamageMin, int32 NewDamageMax);

	bool IsValidGridCell(const FVector2D& Pos, bool bIsStart) const;

	// ************ GETTERS ************
	// Get the unique GameUnit ID
	int32 GetGameUnitID() const;


	TArray<FVector2D> CalculatePath(const FVector2D& EndPos);

	// Get the Player Owner
	int32 GetPlayerOwner() const;

	// Get the grid position of the unit
	FVector2D GetGridPosition() const;

	// Get the type of the Game Unit
	EGameUnitType GetGameUnitType() const;

	// Get the unit's Hit Points
	int32 GetHitPoints() const;

	// Get the unit's Movement Range
	int32 GetMovementRange() const;

	// Get the unit's Attack Range
	int32 GetAttackRange() const;

	// Get the unit's minimum damage value
	int32 GetDamageMin() const;

	// Get the unit's maximum damage value
	int32 GetDamageMax() const;

	// ************ METHODS ************
	// Calculate and return legal moves based on MovementRange.
	// Only horizontal and vertical moves are considered.
	virtual TArray<FVector2D> CalculateLegalMoves();

	// Calcola e restituisce le mosse legali per l'attacco basate su AttackRange.
// Solo spostamenti orizzontali e verticali sono considerati.
	virtual TArray<FVector2D> CalculateAttackMoves() const;


	// Apply damage to the unit; if HitPoints reach 0, the unit is considered dead.
	virtual void TakeDamageUnit(int32 DamageAmount);

	// Returns true if the unit is dead (HitPoints <= 0)
	bool IsDead() const;

	// Nell'header di AGameUnit aggiungi:
	bool bHasMoved;
	bool bHasAttacked;
};



