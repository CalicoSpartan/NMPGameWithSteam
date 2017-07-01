// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Net/UnrealNetwork.h"


APickup::APickup()
{
	//tell engine to replicate this actor
	bReplicates = true;
	//pickups do not need to tick
	PrimaryActorTick.bCanEverTick = false;
	//static mesh actor disables overlap events by default, which we need to re-enable
	GetStaticMeshComponent()->bGenerateOverlapEvents = true;
	if (Role == ROLE_Authority)
	{
		bIsActive = true;
	}


}

void APickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickup, bIsActive);
}

bool APickup::IsActive()
{
	return bIsActive;
}

void APickup::SetActive(bool NewPickupState)
{
	if (Role == ROLE_Authority)
	{
		bIsActive = NewPickupState;
	}
}

void APickup::WasCollected_Implementation()
{
	// log a debug message
	UE_LOG(LogClass, Log, TEXT("APickup::WasCollected_Implementation %s"), *GetName());
}

void APickup::PickedUpBy(APawn * Pawn)
{
	if (Role == ROLE_Authority)
	{
		//store the pawn who picked up the pickup
		PickupInstigator = Pawn;
		//Notify clients of the picked up action
		ClientOnPickedUpBy(Pawn);
	}
}

void APickup::OnRep_IsActive()
{
	
}

void APickup::ClientOnPickedUpBy_Implementation(APawn* Pawn)
{
	//store the pawn who picked up pickup (client
	PickupInstigator = Pawn;
	//fire the blueprint native event, which itself cannot be replicated
	WasCollected();
}
