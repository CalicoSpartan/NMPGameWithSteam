// Fill out your copyright notice in the Description page of Project Settings.

#include "NMPGameGameStateBase.h"
#include "Net/UnrealNetwork.h"

ANMPGameGameStateBase::ANMPGameGameStateBase()
{
	//set default state when state is not currently known
	CurrentState = EBatteryPlayState::EUnknown;
}


void ANMPGameGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANMPGameGameStateBase, PowerToWin);
	DOREPLIFETIME(ANMPGameGameStateBase, CurrentState);
	DOREPLIFETIME(ANMPGameGameStateBase, WinningPlayerName);
}

EBatteryPlayState ANMPGameGameStateBase::GetCurrentState() const
{
	return CurrentState;
}

void ANMPGameGameStateBase::SetCurrentState(EBatteryPlayState NewState)
{
	if (Role == ROLE_Authority)
	{
		CurrentState = NewState;
	}
}

void ANMPGameGameStateBase::OnRep_CurrentState()
{

}