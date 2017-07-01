// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NMPGameCharacter.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "BatteryPickup.h"
#include "Pickup.h"

//////////////////////////////////////////////////////////////////////////
// ANMPGameCharacter

ANMPGameCharacter::ANMPGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//set a base value for the sphere radius
	CollectionSphereRadius = 200.0f;

	//create the collection sphere
	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectionSphere->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	CollectionSphere->bGenerateOverlapEvents = true;
	CollectionSphere->SetSphereRadius(CollectionSphereRadius);

	//set base values for character power
	InitialPower = 2000.0f;
	CurrentPower = InitialPower;

	//base values for controlling movement speed
	BaseSpeed = 10.0f;
	SpeedFactor = 0.75f;


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void ANMPGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANMPGameCharacter, CollectionSphereRadius);
	DOREPLIFETIME(ANMPGameCharacter, InitialPower);
	DOREPLIFETIME(ANMPGameCharacter, CurrentPower);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANMPGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANMPGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANMPGameCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANMPGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANMPGameCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ANMPGameCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ANMPGameCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ANMPGameCharacter::OnResetVR);

	//handle collecting pickups
	PlayerInputComponent->BindAction("CollectPickups",IE_Pressed, this, &ANMPGameCharacter::CollectPickups);
}

void ANMPGameCharacter::CollectPickups()
{
	//ask the server to collect pickups
	ServerCollectPickups();



}

void ANMPGameCharacter::OnRep_CurrentPower()
{
	PowerChangeEffect();


}

bool ANMPGameCharacter::ServerCollectPickups_Validate() //checks that we can collect pickup
{
	return true; //we have nothing to check in this game so we return true always
}
void ANMPGameCharacter::ServerCollectPickups_Implementation()
{
	if (Role == ROLE_Authority)
	{
		//track the total power found in batteries
		float TotalPower = 0.0f;


		// Get all overlapping actors and store them in an array
		TArray<AActor*> CollectedActors;
		CollectionSphere->GetOverlappingActors(CollectedActors);
		// look at each actor inside the collectionsphere
		for (int i = 0; i < CollectedActors.Num(); ++i)
		{
			//if it is a pickup, and it is valid and active
			APickup* const TestPickup = Cast<APickup>(CollectedActors[i]);
			if (TestPickup != NULL && !TestPickup->IsPendingKill() && TestPickup->IsActive())
			{
				// add power if we found a battery
				if (ABatteryPickup* const TestBattery = Cast<ABatteryPickup>(TestPickup))
				{
					TotalPower += TestBattery->GetPower();
				}


				//Collect the pickup and deactivate it
				TestPickup->PickedUpBy(this);
				TestPickup->SetActive(false);
			}
		}	
		// change the character's power based on what we picked up
		if (!FMath::IsNearlyZero(TotalPower,0.001f))
		{
			UpdatePower(TotalPower);
		}
	}
}


float ANMPGameCharacter::GetInitialPower()
{
	return InitialPower;
}

float ANMPGameCharacter::GetCurrentPower()
{
	return CurrentPower;
}

void ANMPGameCharacter::UpdatePower(float DeltaPower)
{
	if (Role == ROLE_Authority)
	{
		// increase (or decrease) current power
		CurrentPower += DeltaPower;
		//set movement speed based on our power level
		GetCharacterMovement()->MaxWalkSpeed = BaseSpeed + SpeedFactor*CurrentPower;

		//Fake the rep notify (listen server does not get the RepNotify automatically)
		OnRep_CurrentPower();
	}
}

void ANMPGameCharacter::OnPlayerDeath_Implementation()
{
	// disconnect controller from pawn
	DetachFromControllerPendingDestroy();

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);
	//ragdoll (init physics)
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();
	GetMesh()->bBlendPhysics = true;
	//disable movement
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);
	//disable collisions on the capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void ANMPGameCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ANMPGameCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ANMPGameCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ANMPGameCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANMPGameCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ANMPGameCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ANMPGameCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
