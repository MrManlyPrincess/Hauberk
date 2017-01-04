// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIInteractable : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 *
 */
class HAUBERK_API IIInteractable
{
	GENERATED_IINTERFACE_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Events")
		void OnInteract();
};
