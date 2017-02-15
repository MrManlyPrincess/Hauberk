// Fill out your copyright notice in the Description page of Project Settings.

#include "Hauberk.h"
#include "Net/UnrealNetwork.h"
#include "StateComponent.h"


// Sets default values for this component's properties
UStateComponent::UStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;
	bAutoActivate = true;
}


// Called when the game starts
void UStateComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UStateComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UStateComponent::OnExitState()
{

}

void UStateComponent::OnEnterState(uint8 CurrState, uint8 PrevState)
{

}

void UStateComponent::SetState(uint8 NewState)
{
	// Notify the old state that we are leaving.
	OnStateExit();

	// Swap the states.
	PreviousState = CurrentState;
	CurrentState = NewState;

	// Notify that the new state has just been swapped,
	OnStateEnter(CurrentState, PreviousState);

	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_UpdateState(NewState);
	}
}

void UStateComponent::OnStateExit_Implementation()
{
	OnExit.Broadcast();
}

void UStateComponent::OnStateEnter_Implementation(uint8 CurrState, uint8 PrevState)
{
	OnEnter.Broadcast(CurrentState, PreviousState);
}

uint8 UStateComponent::GetState() const
{
	return CurrentState;
}

void UStateComponent::OnRepSetCurrentState()
{
	OnRepCurrentState();
}

void UStateComponent::OnRepSetPreviousState()
{
	OnRepPreviousState();
}

void UStateComponent::OnRepCurrentState_Implementation() {}
void UStateComponent::OnRepPreviousState_Implementation() {}


void UStateComponent::Server_UpdateState_Implementation(uint8 NewState)
{
	SetState(NewState);
}

bool UStateComponent::Server_UpdateState_Validate(uint8 NewState)
{
	return true;
}

void UStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UStateComponent, CurrentState);
	DOREPLIFETIME(UStateComponent, PreviousState);
};