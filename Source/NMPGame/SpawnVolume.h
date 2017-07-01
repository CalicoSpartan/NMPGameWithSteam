// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "SpawnVolume.generated.h"

UCLASS()
class NMPGAME_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnVolume();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	//returns the spawn volume box component. FORCEINLINE makes it quicker
	FORCEINLINE class UBoxComponent* GetWhereToSpawn() const { return WhereToSpawn; }

	//find random point within the box component
	UFUNCTION(BlueprintPure,Category = "Spawning")
	FVector GetRandomPointInVolume();

	//toggle spawning of pickups on and off
	UFUNCTION(BlueprintCallable,Category = "Spawning")
	void SetSpawningActive(bool bShouldSpawn);

protected:
	//this is the pickup to spawn
	UPROPERTY(EditAnywhere,Category = "Spawning")
	TSubclassOf<class APickup> WhatToSpawn;

	// access to the timer for recurring spawning
	FTimerHandle SpawnTimer;


	// minimum spawn delay (seconds)
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Spawning")
	float SpawnDelayRangeLow;
	//maximum spawn delay (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SpawnDelayRangeHigh;


	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	//spawn area where pickups will be created
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Spawning", Meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* WhereToSpawn;
	// handle spawning new pickup
	void SpawnPickup();

	//actual time (seconds) before spawning the next pickup
	float SpawnDelay;
};
