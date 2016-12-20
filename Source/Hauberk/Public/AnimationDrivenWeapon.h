// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "CapturedWeaponData.h"
#include "Runtime/Engine/Classes/Animation/AnimMontage.h"
#include "AnimationDrivenWeapon.generated.h"

UCLASS()
class HAUBERK_API AAnimationDrivenWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	/* Sets default values for this actor's properties */
	AAnimationDrivenWeapon();

	/* Whether or not we should record the montage data to the CapturedWeaponData asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Capture Settings")
		bool ShouldRecord;

	/* The display mesh component of the weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		UStaticMeshComponent* Mesh;

	/* Enables tracing between the captured data points. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Playback")
		bool TraceBetweenPoints;

	/* Defines the reference point that the collision element will center on. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Capture Settings")
		UArrowComponent* CapturePoint;

	/* Where the captured animation data will be stored. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Capture Settings")
		UCapturedWeaponData* CapturedWeaponData;

	/* The current owner of the weapon. Used for recording positions and finding the instigator of an attack. */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
		APawn* WeaponOwner;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Blueprint constructor
	virtual void OnConstruction(const FTransform& Transform) override;

	// Call this to start weapon damage, or to begin recording the animation damage path.
	UFUNCTION(BlueprintCallable, Category = "Damage & Recording")
		virtual void StartDamageWindow();

	// Call this to stop weapon damage, or to stop recording the animation damage path.
	UFUNCTION(BlueprintCallable, Category = "Damage & Recording")
		virtual void EndDamageWindow();

	UFUNCTION(BlueprintNativeEvent, Category = Collision)
		void OnCaptureDataPoint(FVector DebugPoint, FRotator DebugRotator);
	void OnCaptureDataPoint_Implementation(FVector DebugPoint, FRotator DebugRotator);

	UFUNCTION(BlueprintNativeEvent, Category = Collision)
		void OnWeaponHitCharacter(FHitResult HitResult, ACharacter* HitCharacter);
	void OnWeaponHitCharacter_Implementation(FHitResult HitResult, ACharacter* HitCharacter);

	UFUNCTION(BlueprintNativeEvent, Category = Collision)
		void OnWeaponHitWorld(FHitResult HitResult);
	void OnWeaponHitWorld_Implementation(FHitResult HitResult);

	UFUNCTION(BlueprintNativeEvent, Category = Collision)
		void OnWeaponTrace(FVector StartPoint, FVector EndPoint);
	void OnWeaponTrace_Implementation(FVector StartPoint, FVector EndPoint);

	UFUNCTION(BlueprintNativeEvent, Category = Collision)
		void OnWeaponTraceConnector(FVector StartPoint, FVector EndPoint);
	void OnWeaponTraceConnector_Implementation(FVector StartPoint, FVector EndPoint);

	UFUNCTION(BlueprintNativeEvent, Category = Collision)
		void OnWeaponTraceStart();
	void OnWeaponTraceStart_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = Collision)
		void OnWeaponTraceComplete();
	void OnWeaponTraceComplete_Implementation();

private:

	// Used to keep track of when a capture started. (Ultimately used in determining plyback length)
	float CaptureStartTime;

	// Used to keep track of whether or not in the tick we should capture data.
	float IsCurrentlyCapturing;

	// The running lists of captured locations and rotations.
	TArray<FVector> CapturedLocations;
	TArray<FRotator> CapturedRotations;

	FVector CurrentDataLocation;
	FRotator CurrentDataRotation;

	TArray<FCollisionBoxProperties> CollisionBoxes;
	TArray<FBoxElement> LastRenderedCollisionBoxes;

	// The timer handle corresponding to the chunk renderer.
	FTimerHandle TimerHandle_ChunkRender;

	// The total amount of chunks to render in this pass.
	int32 TotalChunks;

	// Index of our current chunk;
	int32 CurrentChunk;

	// Called internally to start capturing the damage paths when the weapon is swinging.
	void BeginDamagePathCapture();

	// Called internally to stop capturing the damage paths when the weapon has stopped swinging.
	void EndDamagePathCapture();

	// Called internally to capture a single point of data during a tick.
	void CaptureDataPoint();

	// Called interally to render the captured data points during playback.
	void RenderSavedCollision();

	FCapturedAnimationInfo GetAnimationDataFromMontageName(FName MontageName);

	// This will initiate the rendering of a captured animation chunk.
	void BeginChunkRender(const float SecondsForCurrentChunk);

	// This will stop the rendering of all chunks by clearing the timer and calling ClearChunkRender().
	void StopChunkRender();

	// This will clear the rendering of all chunks
	void ClearChunkRender();

	// This will render a singular chunk.
	void TraceChunk();

	// This will take our current location
	void CheckForWeaponCollision();

	void CalculateBoxCollision(FBoxElement Box, FCollisionQueryParams* TraceParams, FCollisionObjectQueryParams* ObjQueryParams);

	void CalculateFaceCollision(TArray<FVector> PointsToDraw, FCollisionQueryParams* TraceParams, FCollisionObjectQueryParams* ObjQueryParams, bool IsConnectorFace = false);

	void ConnectBoxDataWithCollision(FBoxElement CurrentBox, FBoxElement LastBox, FCollisionQueryParams* TraceParams, FCollisionObjectQueryParams* ObjQueryParams);
};
