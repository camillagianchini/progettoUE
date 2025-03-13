#pragma once

#include "Tile.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "Blueprint/UserWidget.h"
#include "GameField.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReset);

UCLASS()
class PA5316005_API AGameField : public AActor
{
	GENERATED_BODY()

public:
	// Costruttore
	AGameField();

	// Chiamato quando l’istanza della classe è piazzata in editor o spawnata a runtime
	virtual void OnConstruction(const FTransform& Transform) override;

	// Chiamato all’avvio del gioco (dopo BeginPlay)
	virtual void BeginPlay() override;

	static constexpr int32 NOT_ASSIGNED = -1;

	// Restituisce una tile libera casuale; se non ne esistono, ritorna nullptr.
	ATile* GetRandomFreeTile() const;


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

	void SetSelectedTile(FVector2D Position);

	void SetLegalMoves(const TArray<FVector2D>& NewLegalMoves);

	FVector2D GetPosition(const FHitResult& Hit);

	TArray<ATile*>& GetTileArray();

	TMap<FVector2D, ATile*> GetTileMap();

	FVector2D GetSelectedTile() const;

	TArray<FVector2D> GetLegalMoves();

	FVector GetRelativePositionByXYPosition(const int32 InX, const int32 InY) const;

	FVector2D GetXYPositionByRelativeLocation(const FVector& Location) const;

	void GenerateField();

	void GenerateLettersAndNumbers(int32 X, int32 Y);

	template<typename T>
	void GenerateGameUnit(FVector2D Position, int32 Player);

	inline bool IsValidPosition(const FVector2D Position) const;

	void SelectTile(const FVector2D Position);

	void ResetGameStatusField();

	TArray<FVector2D> LegalMoves(FVector2D Position) const;

	TArray<FVector2D> PossibleMoves(FVector2D Position) const;

	void ShowLegalMovesInTheField();

	UFUNCTION(BlueprintCallable)
	void ResetField();
};
