// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	/** Camera */
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
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
		EnhancedInputComponent->BindAction(TurnRate, ETriggerEvent::Triggered, this, &AShooterCharacter::TurnAtRate);
		EnhancedInputComponent->BindAction(LookUpRate, ETriggerEvent::Triggered, this, &AShooterCharacter::LookUpAtRate);
	}

}

