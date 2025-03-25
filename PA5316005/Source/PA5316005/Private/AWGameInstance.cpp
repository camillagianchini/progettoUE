#include "AWGameInstance.h"

void UAWGameInstance::SetTurnMessage(FString Message)
{
	CurrentTurnMessage = Message;
	// Qui potresti anche aggiornare un widget se lo desideri.
}

void UAWGameInstance::SetOpponent(const int32 SelectedOpp)
{
	// Implementa qui la logica per impostare il tipo di avversario o la difficoltà.
	// Per esempio, potresti salvare SelectedOpp in una variabile membro (non mostrata in questo esempio).
}



FString UAWGameInstance::GetTurnMessage()
{
	return CurrentTurnMessage;
}

