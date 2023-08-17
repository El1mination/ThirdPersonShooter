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

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	/** Camera */
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);

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
	GetCharacterMovement()->JumpZVelocity = 350.f;
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

		// Line Trarce
		FHitResult FireHit;
		const FVector Start{ SocketTransform.GetLocation() };

		// Gets X Axis on the BarrelSocket (Forward Direction)
		const FQuat Rotation{ SocketTransform.GetRotation() }; // Rotation of Start
		const FVector RotationAxis{ Rotation.GetAxisX() };

		const FVector End{ Start + RotationAxis * 50'000.f }; // End = Start + Forward Direction * 50'000 units

		FVector BeamEndPoint{ End }; // BeamEndPoint is originally initialized to the length of the LineTrace from the BarrelSocket to 50'000 units forward

		GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
		if (FireHit.bBlockingHit)
		{
			BeamEndPoint = FireHit.Location; // If BeamEndPoint hits an object, it is initialized to the location of FireHit.Location

			// Spawn Impact Particles at Hit Location
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location);
			}
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			Beam->SetVectorParameter(FName("Target"), BeamEndPoint); // Changes Beams End Location (Target) to the BeamEndPoint, shoots beam from SocketTransform to BeandEndPoint
		}
	}

	// Play Fire Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	/** Cast PlayerInputComponent to EnhancedInputComponent */
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AShooterCharacter::MoveForward);
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AShooterCharacter::MoveRight);
		EnhancedInputComponent->BindAction(TurnRateAction, ETriggerEvent::Triggered, this, &AShooterCharacter::TurnAtRate);
		EnhancedInputComponent->BindAction(LookUpRateAction, ETriggerEvent::Triggered, this, &AShooterCharacter::LookUpAtRate);
		//EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &APawn::AddControllerYawInput);
		//EnhancedInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &APawn::AddControllerPitchInput);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpReleaseAction, ETriggerEvent::Triggered, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::FireWeapon);
	}

}

