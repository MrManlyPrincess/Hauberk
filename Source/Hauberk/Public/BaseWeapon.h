// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AnimationDrivenWeapon.h"
#include "Enumerations.h"
#include "Damageable.h"
#include "BaseWeapon.generated.h"

USTRUCT(BlueprintType)
struct FCombo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UAnimMontage* Montage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float StaminaCost;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FDamageMap AdditionalDamage;
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

	UPROPERTY(BlueprintReadWrite, Transient, Category = "Damage")
		TArray<AActor*> DamagedActors;

	UPROPERTY(Transient)
		FCombo LastKnownCombo;

	UPROPERTY(Transient)
		float ComboIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
		TArray<FDamageMap> DamageMaps;
	
	UFUNCTION(BlueprintCallable, Category = "Damage")
		FCombo GetCurrentCombo() const;

	UFUNCTION(BlueprintCallable, Category = "Damage")
		FCombo GetNextCombo(EAttackType AttackType);

	UFUNCTION(BlueprintCallable, Category = "Damage")
		void ResetCombo();

	UFUNCTION(BlueprintCallable, Category = "Damage")
		void ClearDamagedActors();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
