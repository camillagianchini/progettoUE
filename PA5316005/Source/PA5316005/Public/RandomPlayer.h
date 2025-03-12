// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AWGameInstance.h"
#include "PlayerInterface.h"
#include "AWGameMode.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "RandomPlayer.generated.h"

UCLASS()
class PA5316005_API ARandomPlayer : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ARandomPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UAWGameInstance* GameInstance;

	AAWGameMode* GameMode;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;

	void PlaceUnitsRandomly();

	// Funzione per eseguire una mossa casuale (spostamento)
	void MakeRandomMove();

	// Funzione per eseguire un attacco casuale (se disponibili mosse d'attacco)
	void MakeRandomAttack();
};

