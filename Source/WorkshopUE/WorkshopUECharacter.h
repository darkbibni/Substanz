// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GunComponent.h"
#include "Transformable.h"
#include "WorkshopUECharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AWorkshopUECharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Arms;

	/** Gun canon mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UStaticMeshComponent* FP_GunCanon;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* FP_GunTubes;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* shootOrigin;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

public:
	AWorkshopUECharacter();

protected:
	virtual void BeginPlay();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	class USoundBase* FireSound;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	class USoundBase* AbsorbSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UGunComponent* gunComponent;

	/** Default length of the absorb ray. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float rayLength;

protected:
	
	/** Fires a projectile. */
	void OnFire();

	void OnAbsorb();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	//void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;

	TArray<ATransformable*> transformablesUsed;

	void ResetTransformables();

	FVector lastCheckpoint;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Arms; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	void DisplayGun(bool);

	/**
	Equip the gun to the player.
	*/
	UFUNCTION(BlueprintCallable)
	void EquipGun();

	/**
	Unlock a new power to the gun.
	*/
	UFUNCTION(BlueprintCallable)
	void UnlockNewPower(int index);

	/**
	Switch to translate power.
	*/
	void SwitchToPower1();

	/**
	Switch to rotate power.
	*/
	void SwitchToPower2();

	/**
	Switch to scale power
	*/
	void SwitchToPower3();

	/**
	Change power with the mouse wheel.
	*/
	void ChangePower(float value);

	/** Set the new respawn point.
	*/
	UFUNCTION(BlueprintCallable)
	void SetNewCheckpoint(FVector newCheckpoint);

	/** Teleport player to last checkpoint.
	*/
	UFUNCTION(BlueprintCallable)
	void TeleportToLastCheckpoint();

	/** Respawn player.
	*/
	UFUNCTION(BlueprintCallable)
	void KillPlayer();

	/** Pass through a barrer.
	*/
	UFUNCTION(BlueprintCallable)
	void PassThroughBarrer();

	/** Pass through a barrer.
	*/
	UFUNCTION(BlueprintCallable)
	void ExitBarrer();


	/** Add a transformable affected by power.
	*/
	void AddTransformable(ATransformable* transformable);
};

