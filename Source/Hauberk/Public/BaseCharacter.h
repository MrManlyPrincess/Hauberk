// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "Damageable.h"
#include "Targetable.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "Enumerations.h"
#include "BaseCharacter.generated.h"

UCLASS()
class HAUBERK_API ABaseCharacter : public ACharacter, public IDamageable, public ITargetable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:

	// Player Camera properties.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Components|Camera")
		USpringArmComponent* CameraArm;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Components|Camera")
		UCameraComponent* PlayerCamera;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Components|Health")
		UHealthComponent* PlayerHealth;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Components|Stamina")
		UStaminaComponent* PlayerStamina;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Camera")
		float CameraUpdateSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
		FVector CameraDefaultOffset;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Camera")
		FVector CameraLockedOnOffset;


	// LockOn Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Transient, Category = "Lock On", meta = (DisplayName = "Lock Target"))
		APawn* Target_LockOn;

	UPROPERTY(BlueprintReadWrite, Replicated, Transient, Category = "Lock On", meta = (DisplayName = "Is Locked On"))
		bool bIsLockedOn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Lock On")
		float LockOnRange;


public:

	FTimerHandle TimerHandle_Check_ValidTarget;
	uint8 LockTargetInvalidCount;
	uint8 LockTargetInvalidLimit;

#pragma region Character Default Functions
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called when the player reaches the apex of their jump.
	virtual void NotifyJumpApex() override;

	// Used to determine the apex of this characters jump.
	UFUNCTION(BlueprintNativeEvent, Category = "Movement")
		void OnJumpApexReached();
	void OnJumpApexReached_Implementation();

	// Override when landing to implement the apex notification.
	virtual void Landed(const FHitResult& Hit) override;
#pragma endregion


	// Interfaces
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Target")
		FVector GetTargetableLocation();
	virtual FVector GetTargetableLocation_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Lock On")
		bool GetClosestLockableTarget(ELockDirection Direction, ACharacter*& FoundTarget);

	// Called each frame to update the camera's position.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
		void UpdateCamera();

	// Called each frame AFTER the update camera function to add post processing.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
		void AddCameraOffset();

	// Called when needing characters in our viewport.
	UFUNCTION(BlueprintCallable, Category = "Camera|Frustrum")
		bool IsInFrustrum(ACharacter* Character);

	// Used to collect characters in our frustrum, that are not blocked by geometry.
	UFUNCTION(BlueprintCallable, Category = "Camera")
		TArray<ACharacter*> GetCharactersInView();

	// Checks for a collision between our players camera and the target character.
	UFUNCTION(BlueprintCallable, Category = "Collision")
		bool IsCharacterBlockedByGeometry(ACharacter* TargetCharacter) const;

	// Checks to see if the current LockTarget is still a valid target.
	UFUNCTION(BlueprintCallable, Category = "Lock On")
		void CheckIfStillValidTarget();

	// Checks to see if our health is above zero
	UFUNCTION(BlueprintCallable, Category = "Character|Stats")
		bool IsAlive() const;

#pragma region Replication

	UFUNCTION(BlueprintCallable, Category = "Lock On")
		virtual void LockOn();
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void Server_LockOn();
	virtual void Server_LockOn_Implementation();
	virtual bool Server_LockOn_Validate();

	UFUNCTION(BlueprintCallable, Category = "Lock On")
		virtual void Unlock();
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void Server_Unlock();
	virtual void Server_Unlock_Implementation();
	virtual bool Server_Unlock_Validate();

	UFUNCTION(BlueprintCallable, Category = "Lock On")
		virtual void UpdateLockTarget(ACharacter* NewTarget);
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void Server_UpdateLockTarget(ACharacter* NewTarget);
	virtual void Server_UpdateLockTarget_Implementation(ACharacter* NewTarget);
	virtual bool Server_UpdateLockTarget_Validate(ACharacter* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Animation")
		virtual void PlayNetworkAnim(UAnimMontage* Montage);
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void Server_PlayNetworkAnim(UAnimMontage* Montage);
	virtual void Server_PlayNetworkAnim_Implementation(UAnimMontage* Montage);
	virtual bool Server_PlayNetworkAnim_Validate(UAnimMontage* Montage);

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		virtual void Client_PlayNetworkAnim(UAnimMontage* Montage);
	virtual void Client_PlayNetworkAnim_Implementation(UAnimMontage* Montage);
	virtual bool Client_PlayNetworkAnim_Validate(UAnimMontage* Montage);
#pragma endregion
};
