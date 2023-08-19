// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (!ShooterCharacter) // Makes sure ShooterCharacter isn't null
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if (ShooterCharacter)
	{
		// Set Speed
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();

		// Set IsInAir
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		// Set Is Accelerating
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}

		// GetBaseAimRotation Returns a Float Where Turning To 0 is Facing the World X Direction (Forward) - Rotation In Relation to Turn Direction
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		
		// Moving Forward Is 0 When Facing The World X Direction (Forward) - Rotation In Relation To Movement/Velocity
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

		// Gets Difference Between MovementRotation and AimRotation In The Yaw Direction and Stores as Float
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw; // Gets difference Between

		/*FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation %f"), AimRotation.Yaw);
		if (GEngine) { GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Blue, RotationMessage); }*/

		/*FString MovementRotationMessage = FString::Printf(TEXT("Base Movement Rotation %f"), MovementRotation.Yaw);
		if (GEngine) { GEngine->AddOnScreenDebugMessage(2, 0.f, FColor::Blue, MovementRotationMessage); }*/

		/*FString OffsetMessage = FString::Printf(TEXT("Movement Offset Yaw %f"), MovementOffsetYaw);
		if (GEngine) { GEngine->AddOnScreenDebugMessage(2, 0.f, FColor::Blue, OffsetMessage); }*/

	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}
