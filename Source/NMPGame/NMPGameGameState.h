// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "NMPGameGameState.generated.h"

/**
 * 
 */
UCLASS()
class NMPGAME_API ANMPGameGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	ANMPGameGameState();

	//required network scaffolding
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//to track power level requiered to win the game
	UPROPERTY(Replicated,BlueprintReadOnly,Category = "Power")
	float PowerToWin;
	
	
};
