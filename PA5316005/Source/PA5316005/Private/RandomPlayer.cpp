// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomPlayer.h"

// Sets default values
ARandomPlayer::ARandomPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GameInstance = Cast<UAWGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

}

// Called when the game starts or when spawned
void ARandomPlayer::BeginPlay()
{
	Super::BeginPlay();
	GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
	
}

// Called every frame
void ARandomPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARandomPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ARandomPlayer::OnTurn()
{
    // Esempio: l’AI decide se attaccare prima o muoversi prima
    MakeRandomAttack();
    MakeRandomMove();
}

void ARandomPlayer::MakeRandomAttack()
{
    UE_LOG(LogTemp, Log, TEXT("MakeRandomAttack chiamato: implementa la logica di attacco casuale."));
    // Esempio di logica:
    // 1. Trova eventuali nemici a portata
    // 2. Se ne esiste uno, esegui un attacco random
}

void ARandomPlayer::MakeRandomMove()
{
    UE_LOG(LogTemp, Log, TEXT("MakeRandomMove chiamato: implementa la logica di movimento casuale."));
    // Esempio di logica:
    // 1. Ottieni la lista di possibili mosse
    // 2. Scegli una tile casuale tra quelle libere e spostati
}


void ARandomPlayer::OnWin()
{
	GameInstance->SetTurnMessage(TEXT("AI Wins!!"));
	GameInstance->IncrementScoreAiPlayer();
}

void ARandomPlayer::OnLose()
{
    GameInstance->SetTurnMessage(TEXT("AI Loses!!"));
}

