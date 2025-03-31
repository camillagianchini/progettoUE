#include "AWGameInstance.h"

void UAWGameInstance::SetTurnMessage(FString Message)
{
	CurrentTurnMessage = Message;
}

void UAWGameInstance::SetOpponent(const int32 SelectedOpp)
{
	OpponentType = SelectedOpp;
	//UE_LOG(LogTemp, Log, TEXT("Opponent type set to: %d"), OpponentType);
}

FString UAWGameInstance::GetTurnMessage()
{
	return CurrentTurnMessage;
}

