// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"

#define print(text) if (GEngine) {GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Cyan, text); UE_LOG(LogTemp, Log, text); }