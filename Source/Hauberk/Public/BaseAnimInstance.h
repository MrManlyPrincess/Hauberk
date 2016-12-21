// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "Runtime/Engine/Classes/Animation/AnimMontage.h"
#include "BaseAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class HAUBERK_API UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Montage Stats")
	float GetPositionFromMontage(const UAnimMontage* Montage);
};
