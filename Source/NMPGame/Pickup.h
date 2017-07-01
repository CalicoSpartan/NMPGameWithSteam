// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NMPGame.h"
#include "Engine/StaticMeshActor.h"
#include "Pickup.generated.h"

/**
 * 
 */
UCLASS()
class NMPGAME_API APickup : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:
	//set default values
	APickup();
	//required network scaffolding
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// returns whether or not the pickup is active
	UFUNCTION(BlueprintPure, Category = "Pickup")
	bool IsActive();

	//allows other classes to safely change the pickup's activation state
	UFUNCTION(BlueprintCallable, Category = "Pickup")
		void SetActive(bool NewPickupState);
	// function to call when the pickup is collected
	UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
	void WasCollected();
	virtual void WasCollected_Implementation();
	//server side handling of being picked up
	UFUNCTION(BlueprintAuthorityOnly,Category = "Pickup")
	virtual void PickedUpBy(APawn* Pawn);
	
protected:
	//true when the pickup can be used, false when the pickup is deactivated
	UPROPERTY(Replicated = OnRep_IsActive)
	bool bIsActive;
	//this is called whenever bIsActive is updated
	UFUNCTION()
	virtual void OnRep_IsActive();

	// the pawn who picked up the pickup
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Pickup")
	APawn* PickupInstigator;
private:
	// client side handling of being picked up
	UFUNCTION(NetMulticast,Unreliable)
	void ClientOnPickedUpBy(APawn* Pawn);
};
