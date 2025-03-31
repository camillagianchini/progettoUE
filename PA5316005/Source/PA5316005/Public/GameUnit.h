#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameUnit.generated.h"

class AAWGameMode;
class AGameField;

UENUM(BlueprintType)
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
	// Constructor
	AGameUnit();

	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

	// -------------------- Components --------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	// -------------------- Unit Identification --------------------
	static int32 NewGameUnitID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 GameUnitID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerOwner;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D GameUnitGridPosition;

	// -------------------- Unit Stats --------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGameUnitType GameUnitType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 HitPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MovementRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 AttackRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 DamageMin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 DamageMax;

	// -------------------- References --------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AAWGameMode* GameMode;



public:
	
	// ------------- Methods -------------
	virtual TArray<FVector2D> CalculateAttackMoves() const;

	virtual TArray<FVector2D> CalculateLegalMoves();

	TArray<FVector2D> CalculatePath(const FVector2D& EndPos);

	// ------------- Setters -------------
	void SetGameUnitID();

	void SetPlayerOwner(int32 NewPlayerOwner);

	void SetGridPosition(const double InX, const double InY);

	void SetGameUnitType(EGameUnitType NewType);

	void SetHitPoints(int32 NewHitPoints);

	void SetMovementRange(int32 NewRange);

	void SetAttackRange(int32 NewAttackRange);

	void SetDamage(int32 NewDamageMin, int32 NewDamageMax);
	
	// Checks if a given grid cell is valid for movement.
	bool IsValidGridCell(const FVector2D& Pos, bool bIsStart) const;

	// ------------- Getters -------------
	int32 GetGameUnitID() const;

	int32 GetPlayerOwner() const;

	FVector2D GetGridPosition() const;

	EGameUnitType GetGameUnitType() const;

	int32 GetHitPoints() const;

	int32 GetMovementRange() const;

	int32 GetAttackRange() const;

	int32 GetDamageMin() const;

	int32 GetDamageMax() const;


	UFUNCTION(BlueprintCallable)
	void TakeDamageUnit(int32 DamageAmount, AGameUnit* Attacker = nullptr);

	bool IsDead() const;

	// ------------- Action Flags -------------
	bool bHasMoved;
	bool bHasAttacked;
};



