// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "StateComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Common), meta=(BlueprintSpawnableComponent) )
class HAUBERK_API UStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStateComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UPROPERTY(Replicated, Transient, ReplicatedUsing = OnRepSetCurrentState)
		uint8 CurrentState;

	UPROPERTY(Replicated, Transient, ReplicatedUsing = OnRepSetPreviousState)
		uint8 PreviousState;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSM_OnExit);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSM_OnEnter, uint8, CurrState, uint8, PrevState);

	UPROPERTY(BlueprintAssignable, Category = "State")
		FSM_OnExit OnExit;

	UPROPERTY(BlueprintAssignable, Category = "State")
		FSM_OnEnter OnEnter;

	void OnExitState();
	void OnEnterState(uint8 CurrState, uint8 PrevState);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "State")
		void OnStateExit();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "State")
		void OnStateEnter(uint8 CurrState, uint8 PrevState);

	UFUNCTION(BlueprintCallable, Category = "State")
		void SetState(uint8 NewState); 

	UFUNCTION(BlueprintCallable, Category = "State")
		uint8 GetState() const;

	UFUNCTION()
		virtual void OnRepSetCurrentState();

	UFUNCTION()
		virtual void OnRepSetPreviousState();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "State")
		void OnRepCurrentState();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "State")
		void OnRepPreviousState();
};

