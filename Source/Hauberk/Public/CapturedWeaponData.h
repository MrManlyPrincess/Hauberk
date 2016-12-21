// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "CapturedWeaponData.generated.h"

struct FSortIndex {
	int Index;
	float Value;

	inline static bool DistanceSort(const FSortIndex& A, const FSortIndex& B)
	{
		return A.Value < B.Value;
	}
};

struct FPlaneData {
public:
	FPlaneData();
	FPlaneData(FVector A, FVector B, FVector C, FVector D);

	TArray<FVector> Positions;
	FVector GetCenter() const;

	void CalculateCenter();

	static TArray<FPlaneData> GetPlanesFromOpposingPlanes(FPlaneData Plane1, FPlaneData Plane2);
};

struct FCollisionBoxProperties {
public:
	FVector Offset;
	FRotator Rotation;
	FVector Dimensions;
};

struct FBoxElement
{
public:
	TArray<FVector> Vertices;
	FVector GetCenter() const;

	FBoxElement();
	FBoxElement(UBoxComponent* BoxComponent);
	FBoxElement(const FVector Dimensions, const FVector Location, const FRotator Rotation);
	void TranslateBox(const FVector Location, const FRotator Rotation);

	/* Returns box planes in the following order:
	Front,
	Right
	Back,
	Left,
	Top,
	Bottom

	Plane Vertices till be stored in clockwise position.
	*/
	TArray<FPlaneData> GetBoxPlanes();
};


/*
The actual data stored from frame-by-frame capture. Will be used for playback
and rendering of the collision.
*/
USTRUCT()
struct FCapturedAnimationInfo {

	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CaptureData")
		UAnimMontage* Montage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CaptureData")
		float MontageLengthInSeconds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CaptureData")
		TArray<FVector> Locations;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CaptureData")
		TArray<FRotator> Rotations;
};

/*
The captured animation data from the weapon for a given animation.
Includes:
- AnimMontage
- AnimMontage length (For rendering collision at the appropriate time)
- CapturedLocations
- CapturedRotations
*/


UCLASS()
class HAUBERK_API UCapturedWeaponData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		TArray<FCapturedAnimationInfo> CapturedAnimationData;
	
	
};
