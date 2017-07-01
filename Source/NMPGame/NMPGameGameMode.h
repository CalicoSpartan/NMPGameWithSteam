// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NMPGameGameStateBase.h"
#include "NMPGameGameMode.generated.h"

UCLASS(minimalapi)
class ANMPGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANMPGameGameMode();
	//Start the power drain timer
	virtual void BeginPlay() override;

	//get the current rate of decay (% of initial power per second
	UFUNCTION(BlueprintPure,Category = "Power")
	float GetDecayRate();

	//access power level requiered to win game
	UFUNCTION(BlueprintPure, Category = "Power")
	float GetPowerToWinMultiplier();
protected:
	//how many times per second to update the character's power and check game rules
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Power")
	float PowerDrainDelay;

	//Access the timer for recurring power draining
	FTimerHandle PowerDrainTimer;


	// the rate at which characters lose power (% of initialpower per second)
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category = "Power",Meta = (BlueprintProtected = "true"))
	float DecayRate;

	//this is the power level needed to win game
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category = "Power",Meta = (BlueprintProtected = "true"))
	float PowerToWinMultiplier;

	//track the number of players who have ran out of energy and have been eliminated
	int32 DeadPlayerCount;

private:
	//Drains power from characters and update gameplay
	void DrainPowerOverTime();

	//track all the spawn volumes in the level
	TArray<class ASpawnVolume*> SpawnVolumeActors;

	//handfle function calls for when the game transitions between states
	void HandleNewState(EBatteryPlayState NewState);

};



