// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"

AShooterCharacter::AShooterCharacter() :
	// Base Rates For Turning/Looking Up
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	// Aiming/Not Aiming Rates (Controller)
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	// Aiming/Not Aiming Rates (Mouse)
	MouseHipTurnRate(1.f),
	MouseHipLookUpRate(1.f),
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),
	// True When Aiming Weapon
	bAiming(false),
	// Camera FOV Values
	CameraDefaultFOV(0.f),
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(20.f),
	// Crosshair Spread Factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	// Bullet Fire Timer Variables
	ShootTimeDuration(0.05f),
	bFiringBullet(false)
{
	PrimaryActorTick.bCanEverTick = true;

	/** Camera */
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 180.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	/** Don't Rotate When Controller Rotates */
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	/** Character Movement  */
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // Character Moves In Direction of Input
	GetCharacterMovement()->JumpZVelocity = 450.f;
	GetCharacterMovement()->AirControl = 0.1f;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CharacterMappingContext, 0);
		}
	}

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView; //CameraDefaultFOv set to Cameras Default FOV
		CameraCurrentFOV = CameraDefaultFOV;
	}
}

void AShooterCharacter::MoveForward(const FInputActionValue& Value)
{
	const float CurrentValue = Value.Get<float>();
	if (Controller && (CurrentValue != 0.f))
	{
		// Gets the Controllers ForwardDirection and stores it in Direction
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
		const FVector ForwardDirection{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };

		AddMovementInput(ForwardDirection, CurrentValue);
	}
}

void AShooterCharacter::MoveRight(const FInputActionValue& Value)
{
	const float CurrentValue = Value.Get<float>();
	if ((Controller) && (CurrentValue != 0.f))
	{
		// Gets the Controllers ForwardDirection and stores it in Direction
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
		const FVector RightDirection{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };

		AddMovementInput(RightDirection, CurrentValue);
	}
}

void AShooterCharacter::TurnAtRate(const FInputActionValue& Value)
{
	const float Currentvalue = Value.Get<float>();

	AddControllerYawInput(Currentvalue * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(const FInputActionValue& Value)
{
	const float Currentvalue = Value.Get<float>();

	AddControllerPitchInput(Currentvalue * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(const FInputActionValue& Value)
{
	const float CurrentValue = Value.Get<float>();
	float TurnScaleFactor;

	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate; // When Aiming, TurnScaleFactor set to MouseAimingTurnRate
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate; // When Not Aiming, TurnScaleFactor set to MouseHipTurnRate
	}
	AddControllerYawInput(CurrentValue * TurnScaleFactor);
}

void AShooterCharacter::LookUp(const FInputActionValue& Value)
{
	const float CurrentValue = Value.Get<float>();
	float LookUpScaleFactor;

	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate; // When Aiming, LookUpScaleFactor set to MouseAimingLookUpRate
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate; // When Not Aiming, LookUpScaleFactor set to MouseLookUpRate
	}
	AddControllerPitchInput(CurrentValue * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	// Play Fire Sound
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		// Play Muzzle Flash Effect
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh()); // Barrel Socket Transform
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		// Create FVector BeamEnd and Populate it With Hit Information
		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		
		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}
	
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform); // Beam Starts At SocketTransform
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd); // Changes Beams End Location (Target) to the BeamEnd, shoots beam from SocketTransform to BeandEndPoint
				}
			}
		}
	}

	// Play Fire Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	// Start Bullet Fire Timer For Crosshairs
	StartCrosshairBulletFire();
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// Get Viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get Crosshair Location
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f; // Raises Crosshair By 50 Units

	// Project The Crosshair From Screen Space to World Space and Fills Variables With Info
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FHitResult ScreenTraceHit;
		const FVector Start{ CrosshairWorldPosition }; // Start is at Crosshair Position
		const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000 }; // End is Crosshair Position 50'000 Units Forward In The Direction Of Crosshair World Direction

		OutBeamLocation = End;// OutBeamLocation is originially Set to End in Case Line Trace Never Hits Anything

		// Line Trace From CrosshairWorldPosition to CrosshariWorldPosition + 50'000 Units In Forward Direction
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		if (ScreenTraceHit.bBlockingHit)
		{
			// Sets OutBeamLocation to HitLocation
			OutBeamLocation = ScreenTraceHit.Location;
		}

		// Perform a Second Trace From Gun Barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{ MuzzleSocketLocation };
		const FVector WeaponTraceEnd{ OutBeamLocation };
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

		// If Object Between Barrel and OutBeamLocation, Set OutBeamLocation to Object Hit by Barrel Trace
		if (WeaponTraceHit.bBlockingHit)
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;
	}
	return false; // If Deprojection Doesn't Work, Return False
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
	UE_LOG(LogTemp, Warning, TEXT("Pressed"));
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
	UE_LOG(LogTemp, Warning, TEXT("Released"));
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	// Set Current Camera Field Of View
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed); // Interpolates Between CameraCurrentFOV and CameraZoomedFOV Every Frame We Are Aiming
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed); // Interpolates Between CameraCurrentFOV and CameraDefaultFOV Every Frame We Are Not Aiming
	}

	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV); // Set Camera FOV to CameraCurrentFOV
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		// If Aiming, Set Base Rates to Aiming Rates
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		// If Not Aiming, Set Base Rates to Hip Rates
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	/** Crosshair Velocity Factor */

	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };

	FVector Velocity{ GetVelocity() }; // Get Velocity and 0 the Z
	Velocity.Z = 0.f;
	
	// Gets a Value Within A Mapped Range Between WalkSpeedRange and VelocityMultiplierRange Using Velocity. e.g. If Velocity is 300.f and VelocityMultiplierRange is Between 0.f and 1.f, CrosshairVelocityFactor = 0.5f
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	/** Crosshair In Air Factor */

	// Checks If Falling
	if (GetCharacterMovement()->IsFalling())
	{
		// Spread Crosshairs Slowly While In Air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		// Shrink Crosshairs Quickly After Hitting Ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	/** Crosshair Aim Factor */

	if (bAiming)
	{
		// Shrink Crosshairs Fast While Aiming
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.35, DeltaTime, 30.f);
	}
	else
	{
		// Spread Crosshairs Fast While Not Aiming
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	/** Bullet Fire Aim Factor */
	
	// Checks If Firing Bullet
	if (bFiringBullet)
	{
		// Spreads Crosshairs Very Fast While Shooting
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		// Shrinks Crosshairs Very Fast After Shooting Timer Ends (0.05 Seconds)
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 15.f);
	}

	CrosshairSpreadMultiplier = 0.5 + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime); // Interps Zoom Based on If Aiming or Not
	SetLookRates(); // Set BaseTurnRate and BaseLookUpRate Based on aiming
	CalculateCrosshairSpread(DeltaTime); // Calculate Crosshair Spread Multiplier
	
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	/** Cast PlayerInputComponent to EnhancedInputComponent */
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		/** Movement */
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AShooterCharacter::MoveForward);
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AShooterCharacter::MoveRight);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpReleaseAction, ETriggerEvent::Triggered, this, &ACharacter::StopJumping);

		/** Contrtoller */
		EnhancedInputComponent->BindAction(TurnRateAction, ETriggerEvent::Triggered, this, &AShooterCharacter::TurnAtRate);
		EnhancedInputComponent->BindAction(LookUpRateAction, ETriggerEvent::Triggered, this, &AShooterCharacter::LookUpAtRate);

		/** Mouse */
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Turn);
		EnhancedInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &AShooterCharacter::LookUp);

		/** Weapon/Aiming */
		EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::FireWeapon);
		EnhancedInputComponent->BindAction(AimPressedAction, ETriggerEvent::Triggered, this, &AShooterCharacter::AimingButtonPressed);
		EnhancedInputComponent->BindAction(AimReleasedAction, ETriggerEvent::Triggered, this, &AShooterCharacter::AimingButtonReleased);
	}

}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

