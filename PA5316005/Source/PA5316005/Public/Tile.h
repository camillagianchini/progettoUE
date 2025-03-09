// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UENUM()
enum class ETileStatus : uint8
{
	EMPTY UMETA(DisplayName = "Empty"),
	OCCUPIED UMETA(DispalyName = "Occupied"),
};

UCLASS()
class PA5316005_API ATile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATile();

	void SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus);

	ETileStatus GetTileStatus();

	int32 GetOwner();

	void SetGridPosition(const double InX, const double InY);

	FVector2D GetGridPosition();

	UFUNCTION(BlueprintCallable, Category = "Tile")
	void HighlightTile(const FColor& InColor);

	UFUNCTION(BlueprintCallable, Category = "Tile")
	void ResetHighlight();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETileStatus Status;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerOwner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TileGridPosition;

	//public:	
		// Called every frame
		//virtual void Tick(float DeltaTime) override;

};

