#include "AWGameInstance.h"

void UAWGameInstance::SetTurnMessage(FString Message)
{
	CurrentTurnMessage = Message;
	// Qui potresti anche aggiornare un widget se lo desideri.
}

void UAWGameInstance::SetOpponent(const int32 SelectedOpp)
{
	// Imposta il tipo di avversario.
	// Ad esempio: 0 = Random AI, 1 = AStar AI.
	OpponentType = SelectedOpp;
	UE_LOG(LogTemp, Log, TEXT("Opponent type set to: %d"), OpponentType);

	// Se desideri, puoi eseguire ulteriori logiche qui, ad esempio aggiornare variabili di difficoltà o notificare altri sistemi.
}


FString UAWGameInstance::GetTurnMessage()
{
	return CurrentTurnMessage;
}

