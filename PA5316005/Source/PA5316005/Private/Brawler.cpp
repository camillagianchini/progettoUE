#include "Brawler.h"


ABrawler::ABrawler()
{
	// Imposta i parametri predefiniti per il Brawler secondo le specifiche:
	// Movimento: max 6 celle, Range attacco: 1, Danno: 1–6, Vita: 40
	SetGameUnitType(EGameUnitType::BRAWLER);
	SetMovementRange(6);
	SetAttackRange(1);
	SetDamage(1, 6);
	SetHitPoints(40);
}

void ABrawler::BeginPlay()
{
	Super::BeginPlay();

	// Assegna un ID univoco all'unità
	SetGameUnitID();

	// Eventuali altre inizializzazioni specifiche per il Brawler.
}



