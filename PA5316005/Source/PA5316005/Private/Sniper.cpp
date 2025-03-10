#include "Sniper.h"
#include "Brawler.h"
#include "GameUnit.h"
#include "Math/UnrealMathUtility.h"

// Se in futuro vuoi controllare se l'unità è un Brawler, includi l'header Brawler.h
//#include "Brawler.h"

ASniper::ASniper()
{
	// Impostiamo le stats specifiche dello Sniper
	MaxMovement = 3;
	AttackType = EAttackType::Ranged;
	AttackRange = 10;
	DamageMin = 4;
	DamageMax = 8;
	HitPoints = 20;
}

void ASniper::AttackUnit(AGameUnit* TargetUnit)
{
	if (!TargetUnit)
	{
		return;
	}

	// 1) Check distanza (lo Sniper ignora gli ostacoli, ma non la distanza)
	int32 Dist = CalculateDistance(GridPosition, TargetUnit->GridPosition);
	if (Dist > AttackRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sniper: Target is out of range! Dist=%d, AttackRange=%d"), Dist, AttackRange);
		return;
	}

	// 2) Calcolo danno random
	int32 Damage = FMath::RandRange(DamageMin, DamageMax);
	UE_LOG(LogTemp, Log, TEXT("Sniper %s attacks %s for %d damage"),
		*GetName(), *TargetUnit->GetName(), Damage);

	TargetUnit->ReceiveDamage(Damage);

	// 3) Logica di contrattacco:
	//    - Se l'unità attaccata è uno Sniper (qualsiasi distanza)
	//    - Oppure se è un Brawler e dist == 1
	//
	// Per distinguere, potresti castare a "ASniper" o "ABrawler" (se esiste).
	// Se non hai ancora la classe Brawler, assumiamo di controllare un flag
	// oppure usi un "typeid" o un "UClass" check. Ecco un esempio generico:

	bool bIsSniper = (TargetUnit->GetClass()->IsChildOf(ASniper::StaticClass()));
	bool bIsBrawler = (TargetUnit->GetClass()->IsChildOf(ABrawler::StaticClass())); 
	// se avessi una classe Brawler

	if (bIsSniper || bIsBrawler && (Dist == 1))
	{
		int32 CounterDamage = GetCounterAttackDamage(); // random 1..3
		UE_LOG(LogTemp, Log, TEXT("Sniper receives a counterattack of %d damage!"), CounterDamage);
		ReceiveDamage(CounterDamage);
	}
}

int32 ASniper::GetCounterAttackDamage() const
{
	// Contrattacco random [1..3]
	return FMath::RandRange(1, 3);
}



