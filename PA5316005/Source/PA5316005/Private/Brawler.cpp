#include "Brawler.h"

ABrawler::ABrawler()
{
	// Imposta i valori specifici per il Brawler:
	// Movimento massimo: 6 celle
	MaxMovement = 6;

	// Tipo di attacco: Melee (corto raggio)
	AttackType = EAttackType::Melee;

	// Range d'attacco: 1 (solo la cella adiacente)
	AttackRange = 1;

	// Danno: random tra 1 e 6
	DamageMin = 1;
	DamageMax = 6;

	// Punti vita: 40
	HitPoints = 40;
}


