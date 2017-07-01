// Fill out your copyright notice in the Description page of Project Settings.

#include "SpawnVolume.h"
#include "Kismet/KismetMathLibrary.h"
#include "Pickup.h"


// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	if (Role == ROLE_Authority) {
		WhereToSpawn = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
		RootComponent = WhereToSpawn;

		// set some base values for range
		SpawnDelayRangeLow = 1.0f;
		SpawnDelayRangeHigh = 4.5f;

	}


}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();

}

FVector ASpawnVolume::GetRandomPointInVolume()
{
	if (WhereToSpawn != NULL) {
		FVector SpawnOrigin = WhereToSpawn->Bounds.Origin;
		FVector SpawnExtent = WhereToSpawn->Bounds.BoxExtent;
		return UKismetMathLibrary::RandomPointInBoundingBox(SpawnOrigin, SpawnExtent);
	}
	return FVector();
}


void ASpawnVolume::SetSpawningActive(bool bShouldSpawn)
{
	if (Role == ROLE_Authority)
	{
		if (bShouldSpawn)
		{

			//Set the timer to start spawning pickups
			SpawnDelay = FMath::FRandRange(SpawnDelayRangeLow, SpawnDelayRangeHigh);
			GetWorldTimerManager().SetTimer(SpawnTimer, this, &ASpawnVolume::SpawnPickup, SpawnDelay, false);
		}
		else
		{
			//stop spawning
			GetWorldTimerManager().ClearTimer(SpawnTimer);
		}
	}
}

// Called every frame
void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpawnVolume::SpawnPickup()
{
	//if we are the server and we have something to spawn
	if (Role == ROLE_Authority && WhatToSpawn != NULL) {
		//check for valid world
		if (UWorld* const World = GetWorld()) {
			
			//setup any required params/info
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			//where shall we put the pickup?
			FVector SpawnLocation = GetRandomPointInVolume();
			//set a random rotation for the spawned pickup
			FRotator SpawnRotation;
			SpawnRotation.Yaw = FMath::FRand() * 360.f;
			SpawnRotation.Pitch = FMath::FRand() * 360.f;
			SpawnRotation.Roll = FMath::FRand() * 360.f;
			//drop the new pickup into the world
			APickup* const SpawnedPickup = World->SpawnActor<APickup>(WhatToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

			// delay for a bit before the next pickup
			SetSpawningActive(true);
		}
	}
}

