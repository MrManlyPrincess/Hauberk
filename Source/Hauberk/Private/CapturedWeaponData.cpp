// Fill out your copyright notice in the Description page of Project Settings.

#include "Hauberk.h"
#include "CapturedWeaponData.h"

FBoxElement::FBoxElement()
{
}

FBoxElement::FBoxElement(UBoxComponent * BoxComponent)
{
	/*
	FVector Vertices[8] =
	{
	FVector(Min),
	FVector(Min.X, Min.Y, Max.Z),
	FVector(Min.X, Max.Y, Min.Z),
	FVector(Max.X, Min.Y, Min.Z),
	FVector(Max.X, Max.Y, Min.Z),
	FVector(Max.X, Min.Y, Max.Z),
	FVector(Min.X, Max.Y, Max.Z),
	FVector(Max)
	};
	*/
	const FVector BoundsPointMapping[8] =
	{
		FVector(1, 1, 1),
		FVector(1, 1, -1),
		FVector(1, -1, 1),
		FVector(1, -1, -1),
		FVector(-1, 1, 1),
		FVector(-1, 1, -1),
		FVector(-1, -1, 1),
		FVector(-1, -1, -1)
	};

	const FTransform BoxTransform = BoxComponent->GetComponentTransform();

	//Center - Without World Position.
	const FBox Box = BoxComponent->Bounds.GetBox();
	const FVector BoxCenter = Box.GetCenter() - BoxTransform.GetLocation();

	//Extents - Length, Width, Height == BoxExtents * Scale.
	const FVector BoxExtents = BoxComponent->GetUnscaledBoxExtent();

	for (uint8 i = 0; i < 8; i++)
	{
		FVector CalculatedPoint = BoxCenter + ((BoundsPointMapping[i] * BoxExtents));

		FVector WorldSpaceLocation = BoxTransform.TransformPosition(CalculatedPoint);

		Vertices.Add(WorldSpaceLocation);
	}
}

FBoxElement::FBoxElement(const FVector Dimensions, const FVector Location, const FRotator Rotation)
{
	const FVector BoundsPointMapping[8] =
	{
		FVector(1, 1, 1),
		FVector(1, 1, -1),
		FVector(1, -1, 1),
		FVector(1, -1, -1),
		FVector(-1, 1, 1),
		FVector(-1, 1, -1),
		FVector(-1, -1, 1),
		FVector(-1, -1, -1)
	};

	for (uint8 i = 0; i < 8; i++)
	{
		FVector CalculatedPoint = (BoundsPointMapping[i] * Dimensions);

		FVector WorldSpaceLocation = Location + Rotation.RotateVector(CalculatedPoint);

		Vertices.Add(WorldSpaceLocation);
	}

}

void FBoxElement::TranslateBox(const FVector Location, const FRotator Rotation)
{
	for (auto& Position : Vertices)
	{
		Position = Location + Rotation.RotateVector(Position);
	}
}

FVector FBoxElement::GetCenter() const
{
	const FVector CenterDirection = Vertices[7] - Vertices[0];
	const FVector DirectionNormal = CenterDirection.GetSafeNormal();
	const float MinMaxSize = CenterDirection.Size();
	const FVector Center = Vertices[0] + (DirectionNormal * (MinMaxSize * 0.5f));
	return Center;
}

TArray<FPlaneData> FBoxElement::GetBoxPlanes()
{
	TArray<FPlaneData> FinalPlanes;

	FPlaneData FrontPlane;
	FPlaneData RightPlane;
	FPlaneData BackPlane;
	FPlaneData LeftPlane;
	FPlaneData TopPlane;
	FPlaneData BottomPlane;

	FrontPlane.Positions.Add(Vertices[4]);
	FrontPlane.Positions.Add(Vertices[0]);
	FrontPlane.Positions.Add(Vertices[5]);
	FrontPlane.Positions.Add(Vertices[1]);

	RightPlane.Positions.Add(Vertices[0]);
	RightPlane.Positions.Add(Vertices[2]);
	RightPlane.Positions.Add(Vertices[1]);
	RightPlane.Positions.Add(Vertices[3]);

	BackPlane.Positions.Add(Vertices[2]);
	BackPlane.Positions.Add(Vertices[6]);
	BackPlane.Positions.Add(Vertices[3]);
	BackPlane.Positions.Add(Vertices[7]);

	LeftPlane.Positions.Add(Vertices[6]);
	LeftPlane.Positions.Add(Vertices[4]);
	LeftPlane.Positions.Add(Vertices[7]);
	LeftPlane.Positions.Add(Vertices[5]);

	TopPlane.Positions.Add(Vertices[6]);
	TopPlane.Positions.Add(Vertices[2]);
	TopPlane.Positions.Add(Vertices[4]);
	TopPlane.Positions.Add(Vertices[0]);

	BottomPlane.Positions.Add(Vertices[7]);
	BottomPlane.Positions.Add(Vertices[3]);
	BottomPlane.Positions.Add(Vertices[5]);
	BottomPlane.Positions.Add(Vertices[1]);

	FinalPlanes.Add(FrontPlane);
	FinalPlanes.Add(RightPlane);
	FinalPlanes.Add(BackPlane);
	FinalPlanes.Add(LeftPlane);
	FinalPlanes.Add(TopPlane);
	FinalPlanes.Add(BottomPlane);

	return FinalPlanes;
}

FPlaneData::FPlaneData()
{
}

FPlaneData::FPlaneData(FVector A, FVector B, FVector C, FVector D)
{
	Positions.Add(A);
	Positions.Add(B);
	Positions.Add(C);
	Positions.Add(D);
}

TArray<FPlaneData> FPlaneData::GetPlanesFromOpposingPlanes(FPlaneData Plane1, FPlaneData Plane2)
{
	TArray<FPlaneData> FinalPlanes;

	const FVector FrontPlaneCenter = (Plane1.Positions[0] - Plane2.Positions[0]);
	const FVector LeftPlaneCenter = (Plane1.Positions[1] - Plane2.Positions[3]);
	const FVector BackPlaneCenter = (Plane1.Positions[3] - Plane2.Positions[3]);
	const FVector RightPlaneCenter = (Plane1.Positions[0] - Plane2.Positions[2]);

	FinalPlanes.Add(FPlaneData(Plane1.Positions[0], Plane1.Positions[1], Plane2.Positions[0], Plane2.Positions[1]));
	FinalPlanes.Add(FPlaneData(Plane1.Positions[1], Plane1.Positions[2], Plane2.Positions[3], Plane2.Positions[0]));
	FinalPlanes.Add(FPlaneData(Plane1.Positions[2], Plane1.Positions[3], Plane2.Positions[2], Plane2.Positions[3]));
	FinalPlanes.Add(FPlaneData(Plane1.Positions[3], Plane1.Positions[0], Plane2.Positions[1], Plane2.Positions[2]));

	return FinalPlanes;
}

FVector FPlaneData::GetCenter() const
{
	const FVector CenterDirection = Positions[3] - Positions[0];
	const FVector DirectionNormal = CenterDirection.GetSafeNormal();
	const float MinMaxSize = CenterDirection.Size();
	const FVector Center = Positions[0] + (DirectionNormal * (MinMaxSize * 0.5f));
	return Center;
}