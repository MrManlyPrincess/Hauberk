// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HAUBERK_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Health")
		float MaxHealth;
		
private:

	UPROPERTY(Transient, Replicated)
		float Health;

public:

	UFUNCTION(BlueprintCallable, Category = "Health")
		float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
		void IncreaseHealth(float Amount, bool bIsPercentage);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void DecreaseHealth(float Amount, bool bIsPercentage);

	void UpdateHealth(float Amount, bool bIsPercentage);

private:
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void Server_UpdateHealth(float Value, bool bIsPercentage);
	virtual void Server_UpdateHealth_Implementation(float Value, bool bIsPercentage);
	virtual bool Server_UpdateHealth_Validate(float Value, bool bIsPercentage);

};
