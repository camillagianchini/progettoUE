#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sniper.generated.h"

UCLASS()
class PA5316005_API ASniper : public AActor
{
	GENERATED_BODY()

public:
	// Costruttore
	ASniper();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Proprietà dello Sniper
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Unit Stats")
	int32 Health;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Unit Stats")
	int32 MovementRange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Unit Stats")
	int32 AttackRange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Unit Stats")
	int32 MinDamage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Unit Stats")
	int32 MaxDamage;

	// Funzioni principali
	void MoveTo(FVector2D NewPosition);
	void Attack(AActor* Target);
	void TakeDamage(int32 Amount);

	// Controllo se il bersaglio è nel range d'attacco
	bool IsTargetInRange(AActor* Target);

	// Controllo per il contrattacco
	void CounterAttack(AActor* Attacker);
};
