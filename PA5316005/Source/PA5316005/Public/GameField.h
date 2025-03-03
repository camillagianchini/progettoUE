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
	TMap<FVector, ATile*> TileMap;

	float NormalizedCellPaggind;

	static const int32 NOT_ASSIGNED = -1;

	//UPROPERTY(BlueprintAssignable)
	//FOnReset OnResetEvent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Size;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 WinSize;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CellPadding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileSize;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
