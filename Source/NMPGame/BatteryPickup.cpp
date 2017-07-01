// Fill out your copyright notice in the Description page of Project Settings.

#include "BatteryPickup.h"
#include "Net/UnrealNetwork.h"

ABatteryPickup::ABatteryPickup()
{
	//keep movement synced from server to clients
	bReplicateMovement = true;
	//this pickup is physics enabled and should move
	GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->SetSimulatePhysics(true);

	//set base value for the power
	BatteryPower = 200.0f;
}


void ABatteryPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABatteryPickup, BatteryPower);
}

void ABatteryPickup::PickedUpBy(APawn * Pawn)
{
	Super::PickedUpBy(Pawn);

	if (Role == ROLE_Authority)
	{
		// give clients time to play vfx, etc... before destroying the battery
		SetLifeSpan(2.0f);
	}
}

float ABatteryPickup::GetPower()
{
	return BatteryPower;
}
