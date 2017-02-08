// Fill out your copyright notice in the Description page of Project Settings.

#include "Hauberk.h"
#include "BaseCharacter.h"
#include "BaseAnimInstance.h"
#include "UnrealNetwork.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->bNotifyApex = true;
	bReplicates = true;

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Arm"));
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	PlayerHealth = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	PlayerStamina = CreateDefaultSubobject<UStaminaComponent>(TEXT("Stamina Component"));

	CameraArm->SetupAttachment(GetCapsuleComponent());
	PlayerCamera->SetupAttachment(CameraArm);

	CameraArm->bUsePawnControlRotation = true;
	PlayerCamera->bUsePawnControlRotation = true;

	PlayerHealth->SetIsReplicated(true);
	PlayerStamina->SetIsReplicated(true);

	LockOnRange = 2000.f;
	LockTargetInvalidLimit = 3.f;
	LockTargetInvalidCount = 0;
	LockTargetInvalidLimit = 1;
	CameraUpdateSpeed = 5.0f;
	CameraLockedOnOffset = FVector(25.f, 50.f, 50.f);
	CameraDefaultOffset = PlayerCamera->GetRelativeTransform().GetLocation();
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCamera();
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);
}

void ABaseCharacter::NotifyJumpApex()
{
	// Lets blueprints know that we reached the apex of the jump.
	Super::NotifyJumpApex();
	OnJumpApexReached();
}

void ABaseCharacter::OnJumpApexReached_Implementation() {}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
	// Call back to the parent class in case it needs to do any work.
	Super::Landed(Hit);

	// Reset the notify apex to true, otherwise we only get the notification once.
	GetCharacterMovement()->bNotifyApex = true;
}

FVector ABaseCharacter::GetTargetableLocation_Implementation()
{
	return GetActorLocation();
}

bool ABaseCharacter::IsInFrustrum(ACharacter* Character)
{
	ULocalPlayer* LocalPlayer = Cast<APlayerController>(GetController())->GetLocalPlayer();
	if (LocalPlayer != nullptr && LocalPlayer->ViewportClient != nullptr && LocalPlayer->ViewportClient->Viewport)
	{
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			LocalPlayer->ViewportClient->Viewport,
			GetWorld()->Scene,
			LocalPlayer->ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(true)
			);

		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, ViewLocation, ViewRotation, LocalPlayer->ViewportClient->Viewport);
		if (SceneView != nullptr)
		{
			return SceneView->ViewFrustum.IntersectSphere(
				Character->GetActorLocation(), Character->GetSimpleCollisionRadius());
		}
	}

	return false;
}

void ABaseCharacter::UpdateCamera_Implementation()
{	
	// If we're locked on, we want to try to focus on the lock target.
	if (bIsLockedOn)
	{
		if (IsValid(Target_LockOn))
		{
			const ABaseCharacter* _LockTarget = Cast<ABaseCharacter>(Target_LockOn);
			if (_LockTarget)
			{

				// If our lock target is a character, check if they're dead.
				// If they are dead, then unlock.
				if (_LockTarget->IsAlive() == false)
				{
					Unlock();
					return;
				}
			}

			const AController* _Controller = GetController();
			UWorld* _World = GetWorld();
			if (Controller && _World)
			{
				const FRotator _ControlRotation = GetControlRotation();
				const FRotator _CameraToTargetRotation = (Target_LockOn->GetActorLocation() - PlayerCamera->GetComponentLocation()).Rotation();
				const FRotator _PlayerToTargetRotation = (Target_LockOn->GetActorLocation() - GetActorLocation()).Rotation();
				const FRotator _CombinedRotation = FRotator(FMath::Clamp(_CameraToTargetRotation.Pitch, -20.f, 20.f), _PlayerToTargetRotation.Yaw, 0.f);
				const FRotator FinalRotation = FMath::RInterpTo(_ControlRotation, _CombinedRotation, _World->GetDeltaSeconds(), CameraUpdateSpeed);

				Controller->SetControlRotation(FinalRotation);
				CameraArm->SetWorldRotation(FinalRotation);
				AddCameraOffset();
			}
		}
	}
	else
	{
		AddCameraOffset();
	}
}

void ABaseCharacter::AddCameraOffset_Implementation()
{
	UWorld* World = GetWorld();

	if (World)
	{
		if (bIsLockedOn)
		{
			FTransform CameraTransform = PlayerCamera->GetRelativeTransform();
			FMath::VInterpTo(CameraTransform.GetLocation(), CameraLockedOnOffset, World->GetDeltaSeconds(), CameraUpdateSpeed);
		}
		else
		{
			FTransform CameraTransform = PlayerCamera->GetRelativeTransform();
			FMath::VInterpTo(CameraTransform.GetLocation(), CameraDefaultOffset, World->GetDeltaSeconds(), CameraUpdateSpeed);
		}
	}
}

TArray<ACharacter*> ABaseCharacter::GetCharactersInView()
{
	TArray<ACharacter*> CharactersInView = TArray<ACharacter*>();
	UWorld* World = GetWorld();

	for (TActorIterator<ACharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ACharacter* CurrentCharacter = *ActorItr;
		if (CurrentCharacter->GetWorld() != World)
		{
			continue;
		}

		if (
			CurrentCharacter != this &&
			IsInFrustrum(CurrentCharacter) &&
			IsCharacterBlockedByGeometry(CurrentCharacter) == false
			)
		{
			CharactersInView.AddUnique(CurrentCharacter);
		}
	}

	return CharactersInView;
}

bool ABaseCharacter::IsCharacterBlockedByGeometry(ACharacter* TargetCharacter) const
{

	const FVector CameraLocation = PlayerCamera->GetComponentLocation();
	const FVector TargetLocation = TargetCharacter->GetActorLocation();

	FHitResult HitResult;

	FCollisionObjectQueryParams ObjectQueryParams = FCollisionObjectQueryParams();
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Weapon_Trace")), false, this);
	TraceParams.bTraceAsyncScene = true;

	const UWorld* World = GetWorld();

	if (World)
	{
		if (World->LineTraceSingleByObjectType(HitResult, CameraLocation, TargetLocation, ObjectQueryParams, TraceParams))
		{
			return HitResult.bBlockingHit;
		}
	}

	return false;
}

void ABaseCharacter::CheckIfStillValidTarget()
{
	if (bIsLockedOn && IsValid(Target_LockOn) && IsAlive())
	{
		const FVector TargetLocation = Target_LockOn->GetActorLocation();
		if ((TargetLocation - GetActorLocation()).Size() > LockOnRange)
		{
			Unlock();
		}
		else
		{
			ACharacter* LockTarget = Cast<ACharacter>(Target_LockOn);
			if (IsValid(LockTarget))
			{
				if (IsCharacterBlockedByGeometry(LockTarget))
				{
					LockTargetInvalidCount++;
				}
				else
				{
					LockTargetInvalidCount = 0.f;
				}
			}

			if (LockTargetInvalidCount > LockTargetInvalidLimit)
			{
				LockTargetInvalidCount = 0.f;
				Unlock();
			}
		}
	}
	else
	{
		Unlock();
	}
}

bool ABaseCharacter::IsAlive() const
{
	// Simple for now.
	return true;
}

void ABaseCharacter::LockOn()
{
	// If we're already locked on, and we're trying to lock on again, just unlock. 
	// (This makes lock on act like a togglable action)
	if (bIsLockedOn)
	{
		Unlock();
		return;
	}

	bIsLockedOn = true;
	ACharacter* FoundTarget = NULL;

	// We don't have a specific direction, since we're not already locked on.
	// Find the closest target.
	if (GetClosestLockableTarget(ELockDirection::None, FoundTarget))
	{
		UWorld* World = GetWorld();
		if (World)
		{
			// Update our lock target to the found character.
			UpdateLockTarget(FoundTarget);

			// Set a timer to check if we can still see him periodically.
			FTimerManager& TimerManager = World->GetTimerManager();
			TimerManager.ClearTimer(TimerHandle_Check_ValidTarget);
			TimerManager.SetTimer(TimerHandle_Check_ValidTarget, this, &ABaseCharacter::CheckIfStillValidTarget, 0.5f, true);
		}
	}
}

void ABaseCharacter::Server_LockOn_Implementation()
{
	LockOn();
}

bool ABaseCharacter::Server_LockOn_Validate()
{
	return true;
}

void ABaseCharacter::Unlock()
{
	// Set that we aren't locked on anymore and
	// clear our lock target.
	if (bIsLockedOn == false)
	{
		return;
	}

	UpdateLockTarget(NULL);

	// Disable the timer that was checking if the lock target
	// was still a valid target.
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle_Check_ValidTarget);
	}

	// If we aren't the server, call the server function.
	if (Role < ROLE_Authority)
	{
		Server_Unlock();
	}
}

void ABaseCharacter::Server_Unlock_Implementation()
{
	Unlock();
}

bool ABaseCharacter::Server_Unlock_Validate()
{
	return true;
}

bool ABaseCharacter::GetClosestLockableTarget(ELockDirection Direction, ACharacter*& OutTarget)
{
	// Try to find all visible characters in our camera's view.
	const FVector PlayerLocation = GetActorLocation();
	float TargetCenterX = 0.f;
	float DistanceToBeat = 0.f;
	FVector TargetLocation;
	ACharacter* ClosestCharacter = NULL;
	OutTarget = NULL;
	UWorld* World = GetWorld();

	TArray<ACharacter*> Characters = GetCharactersInView();

	if (Direction == ELockDirection::None)
	{
		// We dont have a direction, so just get the closest character.
		DistanceToBeat = LockOnRange;
		for (auto& Character : Characters)
		{
			const FVector TargetCharacterLocation = Character->GetActorLocation();
			const float DistanceToTarget = (TargetCharacterLocation - PlayerLocation).Size();

			// Make sure they're within the LockOn Range
			if (DistanceToTarget < LockOnRange && DistanceToTarget < DistanceToBeat)
			{
				DistanceToBeat = DistanceToTarget;
				ClosestCharacter = Character;
			}
		}

		OutTarget = ClosestCharacter;
		return true;
	}

	else if (Direction == ELockDirection::Left || Direction == ELockDirection::Right)
	{
		// Get our player controller, as we're going to use it to get the screen coordinates
		// of the characters in our view.
		APlayerController* PC = World->GetFirstPlayerController();

		// Get the location of our current lock target, since we'll be using this
		// as our point of reference.
		const FVector LockTargetLocation = Target_LockOn->GetActorLocation();

		FVector2D ScreenLocation = FVector2D();
		FVector2D ViewportSize = FVector2D();

		// Convert the location of our current lock target to an X position.
		if (PC->ProjectWorldLocationToScreen(LockTargetLocation, ScreenLocation))
		{
			// This is now our point of reference.
			TargetCenterX = ScreenLocation.X;

			if (GEngine && GEngine->GameViewport)
			{
				GEngine->GameViewport->GetViewportSize(ViewportSize);
			}

			if (Direction == ELockDirection::Left)
			{
				DistanceToBeat = 0;
			}
			else if (Direction == ELockDirection::Right)
			{
				DistanceToBeat = ViewportSize.X;
			}

			// Loop through the characters in view and determine which is the closest to
			// our desired direction.
			for (auto& Character : Characters)
			{
				// Make sure we skip our current lock target.
				if (Target_LockOn == Character)
				{
					continue;
				}

				TargetLocation = Character->GetActorLocation();
				const float DistanceToTarget = (TargetLocation - PlayerLocation).Size();

				// Make sure this character is in range.
				if (DistanceToTarget < LockOnRange)
				{
					if (PC->ProjectWorldLocationToScreen(LockTargetLocation, ScreenLocation))
					{
						// Check if this character is closest to the left
						if (Direction == ELockDirection::Left)
						{
							if (ScreenLocation.X < TargetCenterX && ScreenLocation.X > DistanceToBeat)
							{
								ScreenLocation.X = DistanceToBeat;
								ClosestCharacter = Character;
							}
						}
						// Check if this character is closest to the right
						else if (Direction == ELockDirection::Right)
						{
							if (ScreenLocation.X > TargetCenterX && ScreenLocation.X < DistanceToBeat)
							{
								ScreenLocation.X = DistanceToBeat;
								ClosestCharacter = Character;
							}
						}
					}
				}
			}

			// If we found a good match, return success with the character we found.
			if (IsValid(ClosestCharacter))
			{
				OutTarget = ClosestCharacter;
				return true;
			}

		}
		return false;
	}

	return false;
}

void ABaseCharacter::UpdateLockTarget(ACharacter* NewTarget)
{
	// Change our lock target pointer to the supplied character
	// and say that we're now locked on.
	if (NewTarget == NULL)
	{
		Target_LockOn = NewTarget;
		bIsLockedOn = false;
	}
	else
	{
		Target_LockOn = NewTarget;
		bIsLockedOn = true;
	}

	// If we aren't the server, call the server function.
	if (Role < ROLE_Authority)
	{
		Server_UpdateLockTarget(NewTarget);
	}
}

void ABaseCharacter::Server_UpdateLockTarget_Implementation(ACharacter* NewTarget)
{
	UpdateLockTarget(NewTarget);
}

bool ABaseCharacter::Server_UpdateLockTarget_Validate(ACharacter* NewTarget)
{
	return true;
}

void ABaseCharacter::PlayNetworkAnim(UAnimMontage* Montage)
{
	if (Role < ROLE_Authority)
	{
		PlayAnimMontage(Montage);
		Server_PlayNetworkAnim(Montage);
	}
	else
	{
		Client_PlayNetworkAnim(Montage);
	}
}

void ABaseCharacter::Server_PlayNetworkAnim_Implementation(UAnimMontage* Montage)
{
	PlayNetworkAnim(Montage);
}

bool ABaseCharacter::Server_PlayNetworkAnim_Validate(UAnimMontage* Montage)
{
	return true;
}

void ABaseCharacter::Client_PlayNetworkAnim_Implementation(UAnimMontage * Montage)
{
	UAnimMontage* CurrentMontage = GetCurrentMontage();

	if (Montage == CurrentMontage)
	{
		UAnimInstance* Test = GetMesh()->GetAnimInstance();
		UBaseAnimInstance* Test2 = Cast<UBaseAnimInstance>(Test);

		float mPosition = Test2->GetPositionFromMontage(Montage);
		float mPlayLength = Montage->GetPlayLength();

		UE_LOG(LogTemp, Log, TEXT("mPosition: %d"), mPosition);
		UE_LOG(LogTemp, Log, TEXT("mPlayLength: %d"), mPlayLength);

		return;
	}

	PlayAnimMontage(Montage);
}

bool ABaseCharacter::Client_PlayNetworkAnim_Validate(UAnimMontage * Montage)
{
	return true;
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ABaseCharacter, Target_LockOn);
	DOREPLIFETIME(ABaseCharacter, bIsLockedOn);

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
};

