// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ShooterCharacter.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	/** Movement */
	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);

	/** Controller */
	void TurnAtRate(const FInputActionValue& Value);
	void LookUpAtRate(const FInputActionValue& Value);

	/** Mouse */
	void Turn(const FInputActionValue& Value);
	void LookUp(const FInputActionValue& Value);

	/** Weapon */
	void FireWeapon();
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);
	void AimingButtonPressed();
	void AimingButtonReleased();
	void CameraInterpZoom(float DeltaTime);
	void SetLookRates(); // Set Turn and LookUp Rate Based on Aiming
	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrosshairBulletFire();

	/** Input Contexts and Actions */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* CharacterMappingContext;

	UPROPERTY(EditAnywhere, BlueprintreadOnly, Category = "Input")
	UInputAction* MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* TurnRateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookUpRateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* TurnAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpReleaseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FireWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AimPressedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AimReleasedAction;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/********** Controller **********/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	// While Not Aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	// While Aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/********** Mouse **********/

	// While Not Aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMIN = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMIN = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	// While Aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMIN = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMIN = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;


	/** Animations */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	/** Sounds */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USoundBase* FireSound;

	/** Particles */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	/** Aiming */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	float CameraDefaultFOV; // Camera FOV by Default

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float CameraZoomedFOV; // Cameras Zoomed FOV

	float CameraCurrentFOV; // Current Field OF View This Frame

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	/** Crosshairs */

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true")) // Determines Spread Of Crosshairs
	float CrosshairSpreadMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true")) // Velocity Component For CrosshairSpread
	float CrosshairVelocityFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true")) // In Air Component For Crosshairs Spread
	float CrosshairInAirFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true")) // Aim Component For Crosshairs Spread
	float CrosshairAimFactor;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true")) // Shooting Component For Crosshairs Spread
	float CrosshairShootingFactor;

	float ShootTimeDuration;

	bool bFiringBullet;

	FTimerHandle CrosshairShootTimer;

public:

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool GetAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;
};
