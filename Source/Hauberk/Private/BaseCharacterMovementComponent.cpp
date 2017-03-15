// Fill out your copyright notice in the Description page of Project Settings.

#include "Hauberk.h"
#include "BaseCharacterMovementComponent.h"

UBaseCharacterMovementComponent::UBaseCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

	SprintSpeedMultiplier = 1.25f;
	SprintAccelerationMultiplier = 1.25f;
	WalkSpeedMultiplier = 0.75f;
	WalkAccelerationMultiplier = 0.75f;
}

//============================================================================================
//Replication
//============================================================================================

//Set input flags on character from saved inputs
void UBaseCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)//Client only
{
	Super::UpdateFromCompressedFlags(Flags);

	//The Flags parameter contains the compressed input flags that are stored in the saved move.
	//UpdateFromCompressed flags simply copies the flags from the saved move into the movement component.
	//It basically just resets the movement component to the state when the move was made so it can simulate from there.
	bWantsToSprint = (Flags&FSavedMove_Character::FLAG_Custom_0) != 0;
	bWantsToWalk = (Flags&FSavedMove_Character::FLAG_Custom_1) != 0;
}

class FNetworkPredictionData_Client* UBaseCharacterMovementComponent::GetPredictionData_Client() const
{
	if (PawnOwner == NULL) {
		return ClientPredictionData;
	}

	if (!ClientPredictionData)
	{
		UBaseCharacterMovementComponent* MutableThis = const_cast<UBaseCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_MyMovement(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void FSavedMove_MyMovement::Clear()
{
	Super::Clear();

	//Clear variables back to their default values.
	bSavedWantsToSprint = false;
	bSavedWantsToWalk = false;
}

//This is where we compress the flags saved in SetMoveFor. We're basically just ORing a bunch of them together.
uint8 FSavedMove_MyMovement::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWantsToSprint)
	{
		Result |= FLAG_Custom_0;
	}

	if (bSavedWantsToWalk)
	{
		Result |= FLAG_Custom_1;
	}

	return Result;
}

bool FSavedMove_MyMovement::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	//This pretty much just tells the engine if it can optimize by combining saved moves. There doesn't appear to be
	//any problem with leaving it out, but it seems that it's good practice to implement this anyways.
	if (bSavedWantsToSprint != ((FSavedMove_MyMovement*)&NewMove)->bSavedWantsToSprint)
	{
		return false;
	}

	if (bSavedWantsToWalk != ((FSavedMove_MyMovement*)&NewMove)->bSavedWantsToWalk)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_MyMovement::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UBaseCharacterMovementComponent* CharMov = Cast<UBaseCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{
		//This is literally just the exact opposite of UpdateFromCompressed flags. We're taking the input
		//from the player and storing it in the saved move.
		bSavedWantsToSprint = CharMov->bWantsToSprint;

		bSavedWantsToWalk = CharMov->bWantsToWalk;
	}
}

void FSavedMove_MyMovement::PrepMoveFor(class ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UBaseCharacterMovementComponent* CharMov = Cast<UBaseCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{
		//This is just the exact opposite of SetMoveFor. It copies the state from the saved move to the movement
		//component before a correction is made to a client.
		

		//Don't update flags here. They're automatically setup before corrections using the compressed flag methods.
	}
}

FNetworkPredictionData_Client_MyMovement::FNetworkPredictionData_Client_MyMovement(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_MyMovement::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_MyMovement());
}


void UBaseCharacterMovementComponent::SetSprinting(bool bSprinting)
{
	bWantsToSprint = bSprinting;
}

void UBaseCharacterMovementComponent::SetWalking(bool bWalking)
{
	bWantsToWalk = bWalking;
}

float UBaseCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	if (bWantsToWalk)
	{
		MaxSpeed *= WalkSpeedMultiplier;
	}

	if (bWantsToSprint)
	{
		MaxSpeed *= SprintSpeedMultiplier;
	}

	return MaxSpeed;
}

float UBaseCharacterMovementComponent::GetMaxAcceleration() const
{
	float MaxAccel = Super::GetMaxAcceleration();

	if (bWantsToWalk)
	{
		MaxAccel *= WalkAccelerationMultiplier;
	}

	if (bWantsToSprint)
	{
		MaxAccel *= SprintAccelerationMultiplier;
	}

	return MaxAccel;
}
