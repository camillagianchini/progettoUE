// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include "GameFramework/Actor.h"
#include "GameField.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReset);

UCLASS()
class PA5316005_API AGameField : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameField();

	UPROPERTY(Transient)
	TArray<ATile*> TileArray;

	UPROPERTY(Transient)
	TMap<FVector2D, ATile*> TileMap;

	float NormalizedCellPaggind;

	static const int32 NOT_ASSIGNED = -1;

	//UPROPERTY(BlueprintAssignable)
	//FOnReset OnResetEvent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Size;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileSize;

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable)
	void ResetField();

	void GenerateField();

	FVector2D GetPosition(const FHitResult& Hit);

	TArray<ATile*>& GetTileArray();

	FVector GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const;

	inline bool IsValidPosition(const FVector2D Position) const;

	TArray<int32> GetLine(const FVector2D Begin, const FVector2D End) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

//public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

};
