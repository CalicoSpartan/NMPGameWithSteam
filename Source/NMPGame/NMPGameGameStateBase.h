// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NMPGameGameStateBase.generated.h"

UENUM(BlueprintType)
enum EBatteryPlayState
{
	EPlaying,
	EGameOver,
	EWon,
	EUnknown
};
/**
 * 
 */
UCLASS()
class NMPGAME_API ANMPGameGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
	
public:
	ANMPGameGameStateBase();

	//required network scaffolding
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//to track power level requiered to win the game
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Power")
		float PowerToWin;
	//this returns the current state of gameplay
	UFUNCTION(BlueprintPure,Category = "Power")
	EBatteryPlayState GetCurrentState() const;
	//transition game into a new play state, Only usable on server
	void SetCurrentState(EBatteryPlayState NewState);

	// rep notify fired on clients to allow for client side changes in gameplay state
	UFUNCTION()
	void OnRep_CurrentState();

	// name of player who won the game
	UPROPERTY(Replicated,BlueprintReadWrite,Category = "Power")
	FString WinningPlayerName;

private:
	// track the current playing state
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	TEnumAsByte<enum EBatteryPlayState> CurrentState;
};
