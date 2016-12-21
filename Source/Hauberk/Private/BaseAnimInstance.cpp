// Fill out your copyright notice in the Description page of Project Settings.

#include "Hauberk.h"
#include "BaseAnimInstance.h"

float UBaseAnimInstance::GetPositionFromMontage(const UAnimMontage* Montage)
{
	return Montage_GetPosition(Montage);
}


