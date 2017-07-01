// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "NMPGame.h"
#include "BatteryPickup.generated.h"

/**
 * 
 */
UCLASS()
class NMPGAME_API ABatteryPickup : public APickup
{
	GENERATED_BODY()

public:
	//Set default values for instances of this class
	ABatteryPickup();

	//requiered network setup. needed because the battery power is replicated
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//server side handling of being picked up by the child class
	UFUNCTION(BlueprintAuthorityOnly, Category = "Pickup")
	virtual void PickedUpBy(APawn* Pawn) override;

	//Public method to access the battery's power level
	float GetPower();

protected:
	//set ammount of power the battery holds
	UPROPERTY(Replicated,EditAnywhere,BlueprintReadWrite,Category = "Power",Meta = (BlueprintProtected = "true"))
	float BatteryPower;
	
	
};
