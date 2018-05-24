// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <memory>
#include "GunComponent.generated.h"

USTRUCT()
struct FPowerStates {

	GENERATED_USTRUCT_BODY()

	TArray<bool> bUnlocked;
	TArray<bool> bAvailable;
	std::shared_ptr<FVector> position;
	std::shared_ptr<FRotator> rotation;
	std::shared_ptr<FVector> scale;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WORKSHOPUE_API UGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGunComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	UStaticMeshComponent* gunTubes;

	/** Projectile classes to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TArray<TSubclassOf<class AWorkshopUEProjectile>> ProjectileClasses;

	/* Current power selected*/
	int currentPower;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void PreviousPower();

	void NextPower();

	void UnlockPower(int index);

	void EquipGun();

	bool bEquipped;

	bool IsEnable();

	/* Change the current selected power. */
	void SwitchToPower(int index);

	bool TryToUsePower();

	void SetPowerAvailable(int index);

	void AbsorbPower();

	/* Update color of the power on the gun.*/
	void SetPowerColor(int index, float value);

	void ResetPowers();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPowerChanged, uint8, index, FVector, color);

	UPROPERTY(BlueprintAssignable, category = "CppFunctions")
	FOnPowerChanged OnPowerChanged;

	FPowerStates powersStates;
};