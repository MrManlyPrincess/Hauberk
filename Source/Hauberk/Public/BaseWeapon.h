// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AnimationDrivenWeapon.h"
#include "BaseWeapon.generated.h"

USTRUCT(BlueprintType)
struct FCombo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UAnimMontage* Montage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float BaseDamageAmount;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float StaminaCost;
};

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	LightAttack,
	ChargeAttack
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class HAUBERK_API ABaseWeapon : public AAnimationDrivenWeapon
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combos")
		TArray<FCombo> LightAttacks;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combos")
		TArray<FCombo> ChargeAttacks;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
		TArray<AActor*> DamagedActors;

	UPROPERTY()
		FCombo LastKnownCombo;

	UPROPERTY()
		float ComboIndex;


	
	UFUNCTION(BlueprintCallable, Category = "Damage")
		FCombo GetCurrentCombo() const;

	UFUNCTION(BlueprintCallable, Category = "Damage")
		FCombo GetNextCombo(EAttackType AttackType);

	UFUNCTION(BlueprintCallable, Category = "Damage")
		void ResetCombo();

	UFUNCTION(BlueprintCallable, Category = "Damage")
		void ClearDamagedActors();

};
