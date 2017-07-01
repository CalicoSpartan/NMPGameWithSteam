// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NMPGame.h"
#include "Engine.h"
#include "Net/UnrealNetwork.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "NMPGameCharacter.generated.h"

UCLASS(config=Game)
class ANMPGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	/** Collection Sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Battery, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* CollectionSphere;
public:
	ANMPGameCharacter();

	//requiered network setup
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	//access to the characters starting power
	UFUNCTION(BlueprintPure, Category = "Power")
	float GetInitialPower();

	//access to the characters current power
	UFUNCTION(BlueprintPure,Category = "Power")
	float GetCurrentPower();

	/**
	* This updates the character's power level
	* @param DeltaPower This is the amount to change power by, can be + or - 
	*/
	UFUNCTION(BlueprintCallable,BlueprintAuthorityOnly,Category = "Power")
	void UpdatePower(float DeltaPower);

	// shut down the pawn and ragdoll it on all clients
	UFUNCTION(NetMultiCast,Reliable) //multicast function happens on server and clients so everyone will see the player ragdoll
	void OnPlayerDeath();

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	//Entry. Called when player presses a key to collect pickups
	UFUNCTION(BlueprintCallable, Category = "Pickups")
		void CollectPickups();

	//called on server to process the collection of pickups
	UFUNCTION(Reliable,Server,WithValidation)
	void ServerCollectPickups();

	// the character's starting power level
	UPROPERTY(Replicated,EditAnywhere,BlueprintReadWrite,Category = "Power", Meta = (BlueprintProtected = "true"))
	float InitialPower;

	//speed when power level is zero
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Power", Meta = (BlueprintProtected = "true"))
	float BaseSpeed;

	//multiplier for controlling current speed depending on power level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power", Meta = (BlueprintProtected = "true"))
	float SpeedFactor;

	//Update character visuals based on current power level
	UFUNCTION(BlueprintImplementableEvent, Category = "Power")
	void PowerChangeEffect();

	



public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	//returns the collectionshpere subobject
	FORCEINLINE class USphereComponent* GetCollectionSphere() const { return CollectionSphere; }

private:
	//variable radius can be changed in editor
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly,Category = "Battery",meta = (AllowPrivateAccess = "true"))
	float CollectionSphereRadius;

	// the character's current power level during gameplay
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPower,VisibleAnywhere,Category = "Power")
	float CurrentPower;

	//power level is updated on clients
	UFUNCTION()
	void OnRep_CurrentPower();

};

