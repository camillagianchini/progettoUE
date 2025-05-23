#pragma once


#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "Tile.h"
#include "GameFramework/Actor.h"

#include "GameField.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReset);


UCLASS()
class PA5316005_API AGameField : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	AGameField();
	
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	static constexpr int32 NOT_ASSIGNED = -1;

	ATile* GetRandomFreeTile() const;

	// --- Properties ---
	UPROPERTY(Transient)
	TArray<ATile*> TileArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FVector2D, ATile*> TileMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<int32, AGameUnit*> GameUnitMap;

	UPROPERTY(EditDefaultsOnly)
	FVector2D SelectedTile;

	UPROPERTY(Transient)
	TArray<FVector2D> LegalMovesArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Size;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CellPadding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NormalizedCellPadding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GameUnitScalePercentage;

	UPROPERTY(BlueprintAssignable)
	FOnReset OnResetEvent;

	UPROPERTY(EditAnywhere)
	UUserWidget* ListOfMovesWidgetRef;


	// --- Methods ---
	void SetSelectedTile(FVector2D Position);

	void SetLegalMoves(const TArray<FVector2D>& NewLegalMoves);

	FVector2D GetPosition(const FHitResult& Hit);

	TArray<ATile*>& GetTileArray();

	TMap<FVector2D, ATile*> GetTileMap();

	FVector2D GetSelectedTile() const;

	void GenerateObstacles(float ObstaclePercentage);

	TArray<FVector2D> GetLegalMoves();

	// --- Methods for coordinate conversion ---
	FVector GetRelativePositionByXYPosition(const int32 InX, const int32 InY) const;

	FVector2D GetXYPositionByRelativeLocation(const FVector& Location) const;

	// --- Field Generation ---
	void GenerateField();

	void GenerateLettersAndNumbers(int32 X, int32 Y);



	template<typename T>
	void GenerateGameUnit(FVector2D Position, int32 Player);

	inline bool IsValidPosition(const FVector2D Position) const;

	void SelectTile(const FVector2D Position);

	void ResetGameStatusField();
	
	TArray<FVector2D> PossibleMoves(FVector2D Position) const;

	void ShowLegalMovesInTheField();

	UFUNCTION(BlueprintCallable)
	void ResetField();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void ShowLegalMovesForUnit(AGameUnit* Unit);

	bool IsGridConnected() const;
	void MoveUnit(AGameUnit* Unit, const FVector2D& NewPos, TFunction<void()> OnMovementFinished);


	UFUNCTION(BlueprintCallable, Category = "Game Field")
	void AttackUnit(AGameUnit* Attacker, const FVector2D& TargetPos);

	void ShowLegalAttackOptionsForUnit(AGameUnit* Unit);

	static FString ConvertGridPosToCellString(const FVector2D& Pos)
	{
		char Letter = 'A' + static_cast<int>(Pos.X);
		int32 Row = static_cast<int>(Pos.Y) + 1;
		return FString::Printf(TEXT("%c%d"), Letter, Row);
	}
};
