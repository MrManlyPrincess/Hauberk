// Fill out your copyright notice in the Description page of Project Settings.

#include "Hauberk.h"
#include "UnrealNetwork.h"
#include "StaminaComponent.h"


// Sets default values for this component's properties
UStaminaComponent::UStaminaComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;
	MaxStamina = 100.f;
	// ...
}


// Called when the game starts
void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	Stamina = MaxStamina;
}


// Called every frame
void UStaminaComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

float UStaminaComponent::GetStamina() const
{
	return Stamina;
}

void UStaminaComponent::IncreaseStamina(float Amount, bool bIsPercentage)
{
	if (Amount < 0)
	{
		//If we got a negative, flip the sign.
		Amount *= -1;
	}

	UpdateStamina(Amount, bIsPercentage);
}

void UStaminaComponent::DecreaseStamina(float Amount, bool bIsPercentage)
{
	if (Amount > 0)
	{
		//If we got a positive, flip the sign.
		Amount *= -1;
	}

	UpdateStamina(Amount, bIsPercentage);
}

void UStaminaComponent::UpdateStamina(float Amount, bool bIsPercentage)
{

	float newAmount = 0.f;

	if (bIsPercentage)
	{
		newAmount = (MaxStamina * Amount) + Stamina;

		if (newAmount > MaxStamina)
		{
			newAmount = MaxStamina;
		}
	}
	else
	{
		newAmount = Stamina + Amount;

		if (newAmount > MaxStamina)
		{
			newAmount = MaxStamina;
		}
	}

	Stamina = newAmount;

	if (Amount > 0)
	{
		OnStaminaIncreased.Broadcast(Amount);
	}
	else
	{
		OnStaminaDecreased.Broadcast(Amount);
	}

	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_UpdateStamina(Amount, bIsPercentage);
	}
}

void UStaminaComponent::Server_UpdateStamina_Implementation(float Value, bool bIsPercentage)
{
	UpdateStamina(Value, bIsPercentage);
}

bool UStaminaComponent::Server_UpdateStamina_Validate(float Value, bool bIsPercentage)
{
	return true;
}

void UStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UStaminaComponent, Stamina);
}
