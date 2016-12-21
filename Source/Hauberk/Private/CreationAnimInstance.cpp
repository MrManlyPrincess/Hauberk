// Fill out your copyright notice in the Description page of Project Settings.

#include "Hauberk.h"
#include "CreationAnimInstance.h"

float UCreationAnimInstance::GetPositionFromMontage(const UAnimMontage* Montage)
{
	return Montage_GetPosition(Montage);
}


