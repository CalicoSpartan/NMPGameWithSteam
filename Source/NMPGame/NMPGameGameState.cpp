// Fill out your copyright notice in the Description page of Project Settings.

#include "NMPGameGameState.h"
#include "Net/UnrealNetwork.h"

ANMPGameGameState::ANMPGameGameState()
{

}

void ANMPGameGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANMPGameGameState, PowerToWin);
}

