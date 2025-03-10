#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.h"
#include "GameUnit.generated.h"

// Tipo di attacco: Ranged (distanza) o Melee (corto raggio)
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Ranged UMETA(DisplayName = "Ranged"),
	Melee  UMETA(DisplayName = "Melee")
};

UCLASS()
class PA5316005_API AGameUnit : public AActor
{
	GENERATED_BODY()

public:
	AGameUnit();

	//------------------------------------------------
	// Statistiche comuni a tutte le unità
	//------------------------------------------------

	// Quante celle (max) può muoversi (ortogonalmente).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 MaxMovement;

	// Tipo di attacco (Ranged o Melee).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	EAttackType AttackType;

	// Range d'attacco (numero di celle).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 AttackRange;

	// Danno minimo e massimo.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 DamageMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 DamageMax;

	// Punti vita dell'unità.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 HitPoints;

	// **Aggiungi questa variabile per risolvere l'errore**:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 UnitOwner;  // 1 = Human, 2 = AI, ecc.

	//------------------------------------------------
	// Riferimenti / Info di posizionamento
	//------------------------------------------------

	// Memorizziamo la posizione di griglia (X, Y) dove si trova l'unità.
	// In alternativa potresti memorizzare un puntatore alla Tile corrente.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Unit Position")
	FVector2D GridPosition;

	// Puntatore al campo di gioco (per poter verificare le tile, ecc.)
	UPROPERTY()
	class AGameField* GameFieldRef;

	//------------------------------------------------
	// Metodi principali
	//------------------------------------------------

	// Sposta l'unità verso una destinazione di griglia (senza diagonali).
	// Esempio di implementazione semplificata.
	UFUNCTION(BlueprintCallable, Category = "Unit Actions")
	virtual bool MoveUnit(const FVector2D& Destination);

	// Attacca un'altra unità.
	UFUNCTION(BlueprintCallable, Category = "Unit Actions")
	virtual void AttackUnit(AGameUnit* TargetUnit);

	// Funzione che calcola il danno e riduce i punti vita.
	virtual void ReceiveDamage(int32 Damage);

protected:
	// Se i punti vita vanno a 0 o meno, l'unità muore.
	virtual void Die();

	// Esempio di funzione di distanza (Manhattan).
	// Se vuoi ignorare ostacoli per l'attacco ranged, userai questa per "range check".
	virtual int32 CalculateDistance(const FVector2D& From, const FVector2D& To) const;
};


