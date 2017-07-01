// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NMPGameGameMode.h"
#include "NMPGameCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "NMPGameGameStateBase.h"
#include "SpawnVolume.h"

ANMPGameGameMode::ANMPGameGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set the type of HUD used in the game
	static ConstructorHelpers::FClassFinder<AHUD> PlayerHUDClass(TEXT("/Game/Blueprints/BP_NMPGameHUD"));
	if (PlayerHUDClass.Class != NULL) {
		HUDClass = PlayerHUDClass.Class;
	}

	//set the type of gamestate used in the game
	GameStateClass = ANMPGameGameStateBase::StaticClass();

	// base decay rate
	DecayRate = 0.02f;

	//base value for how frequently to drain power
	PowerDrainDelay = 0.25f;

	//set base value for the power multiplier
	PowerToWinMultiplier = 1.25f;


	//reset stats
	DeadPlayerCount = 0;




}

float ANMPGameGameMode::GetDecayRate()
{
	return DecayRate;
}

float ANMPGameGameMode::GetPowerToWinMultiplier()
{
	return PowerToWinMultiplier;
}

void ANMPGameGameMode::BeginPlay()
{
	Super::BeginPlay();


	UWorld* World = GetWorld();
	check(World); // replaces if statement. used for if we know this thing should exist. if it doesn't then game crashes. improves overall performance
	ANMPGameGameStateBase* MyGameState = Cast<ANMPGameGameStateBase>(GameState);
	check(MyGameState);

	//reset stats
	DeadPlayerCount = 0;

	//gather all of the spawn volumes and store them for later
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, ASpawnVolume::StaticClass(), FoundActors);
	for (auto Actor : FoundActors)
	{
		if (ASpawnVolume* TestSpawnVol = Cast<ASpawnVolume>(Actor))
		{
			//add the volume to the array and insure it only exists in the array once
			SpawnVolumeActors.AddUnique(TestSpawnVol);
		}
	}

	//transitioning the game to the playing state
	HandleNewState(EBatteryPlayState::EPlaying);

	// Go through all the characters in the game
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It) {
		if (APlayerController* PlayerController = Cast<APlayerController>(*It)) {
			if (ANMPGameCharacter* BatteryCharacter = Cast<ANMPGameCharacter>(PlayerController->GetPawn())) {
				MyGameState->PowerToWin = BatteryCharacter->GetInitialPower() * PowerToWinMultiplier;
				break;
			}
		}
	}
}

void ANMPGameGameMode::DrainPowerOverTime()
{
	// Access the world to get to the players
	UWorld* World = GetWorld();
	check(World); // replaces if statement. used for if we know this thing should exist. if it doesn't then game crashes. improves overall performance
	ANMPGameGameStateBase* MyGameState = Cast<ANMPGameGameStateBase>(GameState);
	check(MyGameState);
	// Go through all the characters in the game
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It) {
		if (APlayerController* PlayerController = Cast<APlayerController>(*It)) {
			if (ANMPGameCharacter* BatteryCharacter = Cast<ANMPGameCharacter>(PlayerController->GetPawn())) {
				if (BatteryCharacter->GetCurrentPower() > MyGameState->PowerToWin) {
					MyGameState->WinningPlayerName = BatteryCharacter->GetName();
					HandleNewState(EBatteryPlayState::EWon);
				}
				else if (BatteryCharacter->GetCurrentPower() > 0) {
					BatteryCharacter->UpdatePower(-PowerDrainDelay*DecayRate*(BatteryCharacter->GetInitialPower()));
				}
				else
				{
					// poor player died.
					BatteryCharacter->OnPlayerDeath();
					
					// see if this is the last player to die and end the game if so
					++DeadPlayerCount;
					if (DeadPlayerCount >= GetNumPlayers())
					{
						HandleNewState(EBatteryPlayState::EGameOver);
					}
				}
			}
		}
	}
	
}


void ANMPGameGameMode::HandleNewState(EBatteryPlayState NewState)
{
	UWorld* World = GetWorld();
	check(World); // replaces if statement. used for if we know this thing should exist. if it doesn't then game crashes. improves overall performance
	ANMPGameGameStateBase* MyGameState = Cast<ANMPGameGameStateBase>(GameState);
	check(MyGameState);
	//only transition if this is actually a new state
	if (NewState != MyGameState->GetCurrentState())
	{
		//update the state, so clients know about the transition
		MyGameState->SetCurrentState(NewState);

		switch (NewState)
		{
		case EBatteryPlayState::EPlaying:
			//activate the spawn volumes
			for (ASpawnVolume* SpawnVol : SpawnVolumeActors)
			{
				SpawnVol->SetSpawningActive(true);
			}
			//start draining power
			GetWorldTimerManager().SetTimer(PowerDrainTimer, this, &ANMPGameGameMode::DrainPowerOverTime, PowerDrainDelay, true);
			break;
		case EBatteryPlayState::EGameOver:
			//deactivate the spawn volumes
			for (ASpawnVolume* SpawnVol : SpawnVolumeActors)
			{
				SpawnVol->SetSpawningActive(false);
			}
			//stop draining power
			GetWorldTimerManager().ClearTimer(PowerDrainTimer);
			break;
		case EBatteryPlayState::EWon:
			//deactivate the spawn volumes
			for (ASpawnVolume* SpawnVol : SpawnVolumeActors)
			{
				SpawnVol->SetSpawningActive(false);
			}
			GetWorldTimerManager().ClearTimer(PowerDrainTimer);
			break;
		default:
		case EBatteryPlayState::EUnknown:
			break;
		}
	}
}


