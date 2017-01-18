// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HAUBERK_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStaminaComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stamina")
		float MaxStamina;

private:
	UPROPERTY(Transient, Replicated)
		float Stamina;

public:
	UFUNCTION(BlueprintCallable, Category = "Stamina")
		float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "Stamina")
		void IncreaseStamina(float Amount, bool bIsPercentage);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
		void DecreaseStamina(float Amount, bool bIsPercentage);

	void UpdateStamina(float Amount, bool bIsPercentage);

private:
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void Server_UpdateStamina(float Value, bool bIsPercentage);
	virtual void Server_UpdateStamina_Implementation(float Value, bool bIsPercentage);
	virtual bool Server_UpdateStamina_Validate(float Value, bool bIsPercentage);
};
