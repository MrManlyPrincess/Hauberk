// Fill out your copyright notice in the Description page of Project Settings.

#include "Hauberk.h"
#include "UnrealNetwork.h"
#include "HealthComponent.h"


// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;
	MaxHealth = 100.f;
	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	Health = MaxHealth;
}


// Called every frame
void UHealthComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

float UHealthComponent::GetHealth() const
{
	return Health;
}

void UHealthComponent::IncreaseHealth(float Amount, bool bIsPercentage)
{
	if (Amount < 0)
	{
		//If we got a negative, flip the sign.
		Amount *= -1;
	}

	UpdateHealth(Amount, bIsPercentage);
}

void UHealthComponent::DecreaseHealth(float Amount, bool bIsPercentage)
{
	if (Amount > 0)
	{
		//If we got a positive, flip the sign.
		Amount *= -1;
	}

	UpdateHealth(Amount, bIsPercentage);
}

void UHealthComponent::UpdateHealth(float Amount, bool bIsPercentage)
{
	float newAmount = 0.f;

	if (bIsPercentage)
	{
		newAmount = (MaxHealth * Amount) + Health;

		if (newAmount > MaxHealth)
		{
			newAmount = MaxHealth;
		}
	}
	else
	{
		newAmount = Health + Amount;

		if (newAmount > MaxHealth)
		{
			newAmount = MaxHealth;
		}
	}

	Health = newAmount;
	
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_UpdateHealth(Amount, bIsPercentage);
	}
}

void UHealthComponent::Server_UpdateHealth_Implementation(float Value, bool bIsPercentage)
{
	UpdateHealth(Value, bIsPercentage);
}

bool UHealthComponent::Server_UpdateHealth_Validate(float Value, bool bIsPercentage)
{
	return true;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UHealthComponent, Health);
}
