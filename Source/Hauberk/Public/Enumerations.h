// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	LightAttack,
	ChargeAttack
};

UENUM(BlueprintType)
enum class EDamageTypes : uint8
{
	Slice,
	Puncture,
	Impact
};

UENUM(BlueprintType)
enum class ELockDirection : uint8
{
	None,
	Left,
	Right
};