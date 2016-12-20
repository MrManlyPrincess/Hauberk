// Fill out your copyright notice in the Description page of Project Settings.

#include "Hauberk.h"
#include "UnrealNetwork.h"
#include "AnimationDrivenWeapon.h"

// Sets default values
AAnimationDrivenWeapon::AAnimationDrivenWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	ShouldRecord = false;
	TraceBetweenPoints = false;

	USceneComponent* SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	CapturePoint = CreateDefaultSubobject<UArrowComponent>(TEXT("CapturePoint"));
	CapturePoint->SetRelativeRotation(FRotator(90.f, 0, 0));

	RootComponent = SceneComp;
	Mesh->SetupAttachment(SceneComp);
	CapturePoint->SetupAttachment(SceneComp);

}

// Called when the game starts or when spawned
void AAnimationDrivenWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Get all the collision boxes from our weapon.
	TArray<UActorComponent*> CollisionElements = GetComponentsByClass(UBoxComponent::StaticClass());

	for (auto& CollisionElement : CollisionElements)
	{
		FCollisionBoxProperties CollisionBoxProperties;
		UBoxComponent* CollisionBox = Cast<UBoxComponent>(CollisionElement);

		CollisionBoxProperties.Offset = CollisionBox->GetRelativeTransform().GetLocation();
		CollisionBoxProperties.Dimensions = CollisionBox->GetScaledBoxExtent();
		CollisionBoxProperties.Rotation = CollisionBox->GetRelativeTransform().GetRotation().Rotator();

		CollisionBoxes.Add(CollisionBoxProperties);
	}
}

// Called every frame
void AAnimationDrivenWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsCurrentlyCapturing)
	{
		CaptureDataPoint();
	}
}

void AAnimationDrivenWeapon::OnConstruction(const FTransform& Transform)
{
}

void AAnimationDrivenWeapon::StartDamageWindow()
{
	if (ShouldRecord)
	{
		// If we are in record mode, then during this window
		// we should start capturing the location of our weapon
		// for playback later on.
		BeginDamagePathCapture();
	}
	else
	{
		// We aren't recording right now, so if we have saved data
		// from a previous recording, we can play it back now.
		RenderSavedCollision();
	}
}

void AAnimationDrivenWeapon::EndDamageWindow()
{
	if (ShouldRecord)
	{
		// If we're in record mode, we should stop the ongoing recording
		// by stopping all running timers, and saving our data to the
		// chosen CapturedWeaponData asset.
		EndDamagePathCapture();
	}
}

void AAnimationDrivenWeapon::BeginDamagePathCapture()
{
	
	if (IsValid(WeaponOwner) && IsValid(Mesh->GetStaticMesh()))
	{
		IsCurrentlyCapturing = true;
		UWorld* World = GetWorld();
		CaptureStartTime = (UGameplayStatics::GetRealTimeSeconds(World) * UGameplayStatics::GetGlobalTimeDilation(World));
		print(TEXT("Started Capture. Logged Start Capture Time"));
	}
	else
	{
		print(TEXT("No Valid Owner or Mesh"));
	}

	CapturedLocations.Empty();
	CapturedRotations.Empty();
}

void AAnimationDrivenWeapon::EndDamagePathCapture()
{
	print(TEXT("Ending Capture"));

	IsCurrentlyCapturing = false;

	if (IsValid(WeaponOwner) && IsValid(Mesh->GetStaticMesh()))
	{
		ACharacter* _Owner = Cast<ACharacter>(WeaponOwner);
		if (IsValid(_Owner))
		{

			print(TEXT("Finalizing Damage Capture"));

			FCapturedAnimationInfo AnimationInfo;
			AnimationInfo.Montage = _Owner->GetCurrentMontage();
			if (IsValid(AnimationInfo.Montage))
			{
				// If we have a previous entry for this montage, get rid of it.
				for (int32 i = 0; i < CapturedWeaponData->CapturedAnimationData.Num(); i++)
				{
					FCapturedAnimationInfo StoredInfo = CapturedWeaponData->CapturedAnimationData[i];
					if (StoredInfo.Montage->GetFName() == AnimationInfo.Montage->GetFName())
					{
						CapturedWeaponData->CapturedAnimationData.RemoveAt(i);
						break;
					}
				}

				// Store the info into our data asset.
				AnimationInfo.Locations = CapturedLocations;
				AnimationInfo.Rotations = CapturedRotations;
				UWorld* World = GetWorld();
				AnimationInfo.MontageLengthInSeconds = (UGameplayStatics::GetRealTimeSeconds(World) * UGameplayStatics::GetGlobalTimeDilation(World)) - CaptureStartTime;
				CapturedWeaponData->CapturedAnimationData.Add(AnimationInfo);
			}
		}
	}
	else
	{
		print(TEXT("No Valid Owner or Mesh"));
	}
}

void AAnimationDrivenWeapon::CaptureDataPoint()
{
	print(TEXT("Capturing Data Point"));

	const FVector OwnerLocation = WeaponOwner->GetActorLocation();
	const FRotator OwnerRotation = WeaponOwner->GetActorRotation();

	if (!OwnerRotation.IsZero())
	{
		WeaponOwner->SetActorRotation(FRotator::ZeroRotator);
	}

	const FVector CapturedDataPoint = CapturePoint->GetComponentLocation() - OwnerLocation;
	const FRotator CapturedRotation = CapturePoint->GetComponentRotation();
	OnCaptureDataPoint(CapturedDataPoint, CapturedRotation);

	CapturedLocations.Add(CapturedDataPoint);
	CapturedRotations.Add(CapturedRotation);
}

void AAnimationDrivenWeapon::RenderSavedCollision()
{
	if (!IsValid(CapturedWeaponData))
	{
		print(TEXT("CapturedWeaponData Asset is not supplied for current weapon. Please pick one from the content browser."));
		return;
	}

	if (!IsValid(WeaponOwner))
	{
		print(TEXT("No Weapon Owner Found."));
		return;
	}

	ACharacter* _Owner = Cast<ACharacter>(WeaponOwner);
	if (IsValid(_Owner))
	{
		UAnimMontage* CurrentlyPlayingMontage = _Owner->GetCurrentMontage();
		if (IsValid(CurrentlyPlayingMontage))
		{
			FCapturedAnimationInfo AnimationInfo = GetAnimationDataFromMontageName(CurrentlyPlayingMontage->GetFName());

			// Check to see that we found a matching montage in our data.
			if (!IsValid(AnimationInfo.Montage))
			{
				print(TEXT("Retreived Animation Info did not have a valid Montage."));
				return;
			}

			CurrentChunk = 0;
			const float SecondsForCurrentChunk = AnimationInfo.MontageLengthInSeconds / AnimationInfo.Locations.Num();
			TotalChunks = AnimationInfo.Locations.Num();

			BeginChunkRender(SecondsForCurrentChunk);
		}
	}
}

FCapturedAnimationInfo AAnimationDrivenWeapon::GetAnimationDataFromMontageName(FName MontageName)
{
	FCapturedAnimationInfo AnimationInfo;

	if (IsValid(CapturedWeaponData) && IsValid(WeaponOwner))
	{
		for (int32 i = 0; i < CapturedWeaponData->CapturedAnimationData.Num(); i++)
		{
			FCapturedAnimationInfo StoredInfo = CapturedWeaponData->CapturedAnimationData[i];
			if (StoredInfo.Montage->GetFName() == MontageName)
			{
				AnimationInfo = StoredInfo;
				break;
			}
		}
	}

	return AnimationInfo;
}

void AAnimationDrivenWeapon::BeginChunkRender(const float SecondsForCurrentChunk)
{
	UWorld* World = GetWorld();

	if (World)
	{
		FTimerManager& TimerManager = World->GetTimerManager();

		TimerManager.ClearTimer(TimerHandle_ChunkRender);

		TimerManager.SetTimer(TimerHandle_ChunkRender, this, &AAnimationDrivenWeapon::TraceChunk, SecondsForCurrentChunk, true);
	}
}

void AAnimationDrivenWeapon::StopChunkRender()
{
	UWorld* World = GetWorld();

	if (World)
	{
		FTimerManager& TimerManager = World->GetTimerManager();

		TimerManager.ClearTimer(TimerHandle_ChunkRender);

		TimerManager.SetTimer(TimerHandle_ChunkRender, this, &AAnimationDrivenWeapon::ClearChunkRender, 1.f, false, 1.f);
	}
}

void AAnimationDrivenWeapon::ClearChunkRender()
{
	UWorld* World = GetWorld();

	if (World)
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle_ChunkRender);
	}

	OnWeaponTraceComplete();

	LastRenderedCollisionBoxes.Empty();
}

void AAnimationDrivenWeapon::TraceChunk()
{
	if (CurrentChunk >= TotalChunks)
	{
		ClearChunkRender();
		return;
	}

	ACharacter* _Owner = Cast<ACharacter>(WeaponOwner);
	if (IsValid(_Owner))
	{
		UAnimMontage* CurrentlyPlayingMontage = _Owner->GetCurrentMontage();
		if (IsValid(CurrentlyPlayingMontage))
		{
			FCapturedAnimationInfo AnimationInfo = GetAnimationDataFromMontageName(CurrentlyPlayingMontage->GetFName());

			if (!IsValid(AnimationInfo.Montage))
			{
				return;
			}

			if (AnimationInfo.Locations.Num() == 0 || AnimationInfo.Rotations.Num() == 0)
			{
				return;
			}

			CurrentDataLocation = AnimationInfo.Locations[CurrentChunk];
			CurrentDataRotation = AnimationInfo.Rotations[CurrentChunk];

			//if (GEngine)
			//{
			//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Rendering Chunk: %d of %d"), CurrentChunk, TotalChunks));
			//}

			CheckForWeaponCollision();
		}
	}

	CurrentChunk++;

	if (CurrentChunk >= TotalChunks)
	{
		ClearChunkRender();
	}
}

void AAnimationDrivenWeapon::CheckForWeaponCollision()
{
	const FVector OwnerLocation = WeaponOwner->GetActorLocation();
	const FRotator OwnerRotation = WeaponOwner->GetActorRotation();

	// Determines the properties of the trace.
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Weapon_Trace")), false, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.AddIgnoredActor(WeaponOwner);

	FCollisionObjectQueryParams ObjQueryParams = FCollisionObjectQueryParams();
	ObjQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	bool DidTraceBetweenPoints = false;
	const FVector CapturePointOffsetFromOwner = CapturePoint->GetComponentLocation() - OwnerLocation;

	for (int32 i = 0; i < CollisionBoxes.Num(); i++)
	{

		// Get the properties from our current collision box index. Which we'll use to build our FBoxElement.
		FCollisionBoxProperties CollisionBoxProperties = CollisionBoxes[i];

		// Get the scaled dimensions from our collision box.
		const FVector ColBoxDimensions = CollisionBoxProperties.Dimensions;

		// Rotate our CurrentDataRotation (our base rotation) by our collision boxes rotation
		const FRotator ColBoxRotation = (CurrentDataRotation.Quaternion() * CollisionBoxProperties.Rotation.Quaternion()).Rotator();

		// Determine the world location.
		// This should be the same location as our captured data, but with the offset of the collision box applied.
		// Include the offset by rotating the relative offset of the collision box by our captured data rotation then adding it
		// to our captured data rotation.
		const FVector ColBoxLocation = (CurrentDataLocation + CurrentDataRotation.RotateVector(CollisionBoxProperties.Offset));

		// Build our FBoxElement with the dimensions, calculated location, and captured data rotation.
		FBoxElement CollisionBoxElement = FBoxElement(ColBoxDimensions, ColBoxLocation, ColBoxRotation);

		// Now that it will display correctly at the origin, translate it to our current players postion and rotation.
		CollisionBoxElement.TranslateBox(OwnerLocation, OwnerRotation);

		CalculateBoxCollision(CollisionBoxElement, &TraceParams, &ObjQueryParams);

		// If we need to trace between points, add this entry to our last known rendered collision boxes
		// these are necessary to draw between the current and last collision boxes.
		if (TraceBetweenPoints)
		{

			if (LastRenderedCollisionBoxes.Num() < CollisionBoxes.Num())
			{
				LastRenderedCollisionBoxes.Add(CollisionBoxElement);
			}
			else
			{
				FBoxElement LastCollisionBoxElement = FBoxElement(LastRenderedCollisionBoxes[i]);
				ConnectBoxDataWithCollision(CollisionBoxElement, LastCollisionBoxElement, &TraceParams, &ObjQueryParams);
				LastRenderedCollisionBoxes.Add(CollisionBoxElement);
				DidTraceBetweenPoints = true;
			}
		}

	}

	if (DidTraceBetweenPoints)
	{
		// If we did trace between our collision boxes, then remove the first 
		// entries in our list, now that the newer entries were added.
		for (int32 i = CollisionBoxes.Num() - 1; i >= 0; --i)
		{
			LastRenderedCollisionBoxes.RemoveAt(i, 1, true);
		}
	}
}

void AAnimationDrivenWeapon::CalculateBoxCollision(FBoxElement Box, FCollisionQueryParams * TraceParams, FCollisionObjectQueryParams * ObjQueryParams)
{

	OnWeaponTraceStart();

	TArray<FPlaneData> Planes = Box.GetBoxPlanes();

	for (int8 i = 0; i < 6; i++)
	{
		CalculateFaceCollision(Planes[i].Positions, TraceParams, ObjQueryParams);
	}
}

void AAnimationDrivenWeapon::CalculateFaceCollision(TArray<FVector> PointsToDraw, FCollisionQueryParams * TraceParams, FCollisionObjectQueryParams * ObjQueryParams, bool IsConnectorFace)
{
	UWorld* World = GetWorld();
	if (World)
	{

		//Get the 4 Points we need to draw.
		const FVector Point1 = PointsToDraw[0];
		const FVector Point2 = PointsToDraw[1];
		const FVector Point3 = PointsToDraw[2];
		const FVector Point4 = PointsToDraw[3];

		FHitResult HitResult1;
		FHitResult HitResult2;
		FHitResult HitResult3;
		FHitResult HitResult4;

		// 1 - 3
		if (IsConnectorFace)
		{
			OnWeaponTraceConnector(Point1, Point3);
		}
		else
		{
			OnWeaponTrace(Point1, Point3);
		}

		if (World->LineTraceSingleByObjectType(HitResult1, Point1, Point3, *ObjQueryParams, *TraceParams))
		{
			AActor* HitActor = HitResult1.GetActor();
			if (HitActor != nullptr)
			{
				if (HitActor->IsA(ACharacter::StaticClass()))
				{
					ACharacter* HitCharacter = Cast<ACharacter>(HitResult1.GetActor());
					OnWeaponHitCharacter(HitResult1, HitCharacter);
				}
				else
				{
					OnWeaponHitWorld(HitResult1);
				}
			}
		}

		// 3 - 4
		if (IsConnectorFace)
		{
			OnWeaponTraceConnector(Point3, Point4);
		}
		else
		{
			OnWeaponTrace(Point3, Point4);
		}

		if (World->LineTraceSingleByObjectType(HitResult2, Point3, Point4, *ObjQueryParams, *TraceParams))
		{
			AActor* HitActor = HitResult2.GetActor();
			if (HitActor != nullptr)
			{
				if (HitActor->IsA(ACharacter::StaticClass()))
				{
					ACharacter* HitCharacter = Cast<ACharacter>(HitResult2.GetActor());
					OnWeaponHitCharacter(HitResult2, HitCharacter);
				}
				else
				{
					OnWeaponHitWorld(HitResult2);
				}
			}
		}

		// 4 - 2
		if (IsConnectorFace)
		{
			OnWeaponTraceConnector(Point4, Point2);
		}
		else
		{
			OnWeaponTrace(Point4, Point2);
		}

		if (World->LineTraceSingleByObjectType(HitResult3, Point4, Point2, *ObjQueryParams, *TraceParams))
		{
			AActor* HitActor = HitResult3.GetActor();
			if (HitActor != nullptr)
			{
				if (HitActor->IsA(ACharacter::StaticClass()))
				{
					ACharacter* HitCharacter = Cast<ACharacter>(HitResult3.GetActor());
					OnWeaponHitCharacter(HitResult3, HitCharacter);
				}
				else
				{
					OnWeaponHitWorld(HitResult3);
				}
			}
		}

		// 2 - 1
		if (IsConnectorFace)
		{
			OnWeaponTraceConnector(Point2, Point1);
		}
		else
		{
			OnWeaponTrace(Point2, Point1);
		}

		if (World->LineTraceSingleByObjectType(HitResult4, Point2, Point1, *ObjQueryParams, *TraceParams))
		{
			AActor* HitActor = HitResult4.GetActor();
			if (HitActor != nullptr)
			{
				if (HitActor->IsA(ACharacter::StaticClass()))
				{
					ACharacter* HitCharacter = Cast<ACharacter>(HitResult4.GetActor());
					OnWeaponHitCharacter(HitResult4, HitCharacter);
				}
				else
				{
					OnWeaponHitWorld(HitResult4);
				}
			}
		}

		// Perform a tessalted trace.
		FHitResult HitResult5;
		if (IsConnectorFace)
		{
			OnWeaponTraceConnector(Point1, Point4);
		}
		else
		{
			OnWeaponTrace(Point1, Point4);
		}

		if (World->LineTraceSingleByObjectType(HitResult5, Point1, Point4, *ObjQueryParams, *TraceParams))
		{
			AActor* HitActor = HitResult5.GetActor();
			if (HitActor != nullptr)
			{
				if (HitActor->IsA(ACharacter::StaticClass()))
				{
					ACharacter* HitCharacter = Cast<ACharacter>(HitResult5.GetActor());
					OnWeaponHitCharacter(HitResult5, HitCharacter);
				}
				else
				{
					OnWeaponHitWorld(HitResult5);
				}
			}
		}
	}
}

void AAnimationDrivenWeapon::ConnectBoxDataWithCollision(FBoxElement CurrentBox, FBoxElement LastBox, FCollisionQueryParams * TraceParams, FCollisionObjectQueryParams * ObjQueryParams)
{
	// Find the half the distance from the current box to the last box.
	// Find the closest points on the current box to the halfway point we found.
	// Draw a connector on that plane.

	if (!TraceBetweenPoints)
	{
		return;
	}

	TArray<FPlaneData> CurrPlanes = CurrentBox.GetBoxPlanes();
	TArray<FPlaneData> LastPlanes = LastBox.GetBoxPlanes();

	//OnImportantPointRender(CurrentBox.Center);
	//OnImportantPointRender(LastBox.Center);

	const FVector Distance = CurrentBox.GetCenter() - LastBox.GetCenter();
	const FVector Direction = Distance.GetSafeNormal();

	const FVector HalfwayPoint = LastBox.GetCenter() + (Direction * (Distance.Size() * 0.5f));

	//OnImportantPointRender(HalfwayPoint);

	TArray<FSortIndex> CurrBoxClosestIndexes;
	TArray<FSortIndex> LastBoxClosestIndexes;

	// Loop through the Planes on both boxes, and see which center point is the closest to the halfway point of both boxes.
	for (int8 i = 0; i < CurrPlanes.Num(); i++)
	{
		FPlaneData CurrBoxPlane = CurrPlanes[i];
		FPlaneData LastBoxPlane = LastPlanes[i];

		FSortIndex CurrBoxPlaneIndex;
		FSortIndex LastBoxPlaneIndex;

		CurrBoxPlaneIndex.Value = (CurrBoxPlane.GetCenter() - HalfwayPoint).Size();
		CurrBoxPlaneIndex.Index = i;

		LastBoxPlaneIndex.Value = (LastBoxPlane.GetCenter() - HalfwayPoint).Size();
		LastBoxPlaneIndex.Index = i;

		CurrBoxClosestIndexes.Add(CurrBoxPlaneIndex);
		LastBoxClosestIndexes.Add(LastBoxPlaneIndex);
	}

	// Loop through the Last Box and find the closest points to the halfway point.
	CurrBoxClosestIndexes.Sort(FSortIndex::DistanceSort);
	LastBoxClosestIndexes.Sort(FSortIndex::DistanceSort);

	FPlaneData ClosestCurrBoxPlane = CurrPlanes[CurrBoxClosestIndexes[0].Index];
	FPlaneData ClosestLastBoxPlane = LastPlanes[LastBoxClosestIndexes[0].Index];

	//OnImportantPointRender(ClosestCurrBoxPlane.Center);
	//OnImportantPointRender(ClosestLastBoxPlane.Center);

	TArray<FPlaneData> Connectors = FPlaneData::GetPlanesFromOpposingPlanes(ClosestCurrBoxPlane, ClosestLastBoxPlane);

	for (int32 i = 0; i < Connectors.Num(); i++)
	{
		CalculateFaceCollision(Connectors[i].Positions, TraceParams, ObjQueryParams, true);
	}
}

void AAnimationDrivenWeapon::OnWeaponHitCharacter_Implementation(FHitResult HitResult, ACharacter* HitCharacter)
{
}

void AAnimationDrivenWeapon::OnWeaponHitWorld_Implementation(FHitResult HitResult)
{
}

void AAnimationDrivenWeapon::OnWeaponTrace_Implementation(FVector StartPoint, FVector EndPoint)
{
}

void AAnimationDrivenWeapon::OnWeaponTraceConnector_Implementation(FVector StartPoint, FVector EndPoint)
{
}

void AAnimationDrivenWeapon::OnWeaponTraceStart_Implementation()
{
}

void AAnimationDrivenWeapon::OnWeaponTraceComplete_Implementation()
{
}

void AAnimationDrivenWeapon::OnCaptureDataPoint_Implementation(FVector DebugPoint, FRotator DebugRotator)
{
}

void AAnimationDrivenWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAnimationDrivenWeapon, WeaponOwner);
}