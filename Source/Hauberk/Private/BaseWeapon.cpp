// Fill out your copyright notice in the Description page of Project Settings.

#include "Hauberk.h"
#include "BaseWeapon.h"

FCombo ABaseWeapon::GetCurrentCombo() const
{
	return LastKnownCombo;
}

FCombo ABaseWeapon::GetNextCombo(EAttackType AttackType)
{
	FCombo Combo;

	switch (AttackType)
	{
		case EAttackType::LightAttack:
			if (LightAttacks.Num() > 0)
			{
				Combo = LightAttacks[ComboIndex];
				ComboIndex++;
				if (ComboIndex >= LightAttacks.Num())
				{
					ComboIndex = 0;
				}
				LastKnownCombo = Combo;
			}
			break;

		case EAttackType::ChargeAttack:

			if (ChargeAttacks.Num() > 0)
			{
				Combo = ChargeAttacks[ComboIndex];
				ComboIndex++;

				if (ComboIndex >= ChargeAttacks.Num())
				{
					ComboIndex = 0;
				}

				LastKnownCombo = Combo;
			}
			break;
	}

	return Combo;
}

void ABaseWeapon::ResetCombo()
{
	ComboIndex = 0;
	ClearDamagedActors();
}

void ABaseWeapon::ClearDamagedActors()
{
	if (DamagedActors.Num() > 0)
	{
		DamagedActors.Empty();
	}
}
