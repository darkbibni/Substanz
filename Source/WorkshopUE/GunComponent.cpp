// Fill out your copyright notice in the Description page of Project Settings.

#include "GunComponent.h"
#include "Engine.h"

// Sets default values for this component's properties
UGunComponent::UGunComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Gun is not equipped by default
	bEquipped = false;

	// Setup gun powers.
	{
		powersStates.bUnlocked.Init(false, 3);
		powersStates.bAvailable.Init(false, 3);

		powersStates.position = std::make_shared<FVector>(FVector::ZeroVector);
		powersStates.rotation = std::make_shared<FRotator>(FRotator::ZeroRotator);
		powersStates.scale = std::make_shared<FVector>(FVector::ZeroVector);
	}

	currentPower = -1;
}


// Called when the game starts
void UGunComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UGunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Rotate gun tubes continuously.
	/*
	if (gunTubes != nullptr) {
		FRotator currentRot = gunTubes->RelativeRotation;
		currentRot.Yaw -= DeltaTime * 100.0f;
		gunTubes->SetRelativeRotation(currentRot);
	}
	*/
}

void UGunComponent::SwitchToPower(int index)
{
	if (index == -1 || !bEquipped) {
		return;
	}

	// Check if power is unlocked and power is not already selected.
	if (powersStates.bUnlocked[index] && currentPower != index) {
		currentPower = index;
		
		FRotator currentRot = gunTubes->RelativeRotation;
		currentRot.Yaw = 60.0f - index*120.0f;
		gunTubes->SetRelativeRotation(currentRot);
	}
}

void UGunComponent::PreviousPower()
{
	bool powerIndexFound = false;

	int previousPowerIndex = currentPower;
	int timeout = 0;

	do {
		previousPowerIndex = previousPowerIndex - 1;

		if (previousPowerIndex < 0) {
			previousPowerIndex = 2;
		}

		if (powersStates.bUnlocked[previousPowerIndex]) {
			powerIndexFound = true;
		}

		timeout++;

	} while (!powerIndexFound && timeout < 3);

	SwitchToPower(previousPowerIndex);
}

void UGunComponent::NextPower()
{
	bool powerIndexFound = false;

	int nextPowerIndex = currentPower;
	int timeout = 0;

	do {
		nextPowerIndex = (nextPowerIndex + 1) % 3;

		if (powersStates.bUnlocked[nextPowerIndex]) {
			powerIndexFound = true;
		}

		timeout++;

	} while (!powerIndexFound && timeout < 3);

	SwitchToPower(nextPowerIndex);
}

void UGunComponent::EquipGun()
{
	bEquipped = true;

	// Unlock first power and select it.
	UnlockPower(0);
	currentPower = 0;
	SwitchToPower(0);
}

void UGunComponent::UnlockPower(int index)
{
	// Unlock this power ans set it available.
	powersStates.bUnlocked[index] = true;
	powersStates.bAvailable[index] = false;

	SetPowerColor(index, 0.15f);
}

bool UGunComponent::IsEnable() {
	return bEquipped && currentPower != -1;
}

bool UGunComponent::TryToUsePower() {

	if (currentPower == -1 || !powersStates.bAvailable[currentPower]) {
		return false;
	}

	SetPowerColor(currentPower, 0.15f);
	
	powersStates.bAvailable[currentPower] = false;

	return true;
}

void UGunComponent::AbsorbPower() {

	if (currentPower == -1) {
		return;
	}

	SetPowerAvailable(currentPower);
}

void UGunComponent::SetPowerColor(int index, float value) {
	
	FVector color(
		index == 0 ? value : 0.0,
		index == 1 ? value : 0.0,
		index == 2 ? value : 0.0
	);
	OnPowerChanged.Broadcast(index, color);
}

void UGunComponent::SetPowerAvailable(int index) {

	if (index < 0) {
		return;
	}

	powersStates.bAvailable[index] = true;

	SetPowerColor(index, 1.0f);
}

void UGunComponent::ResetPowers() {
	
	for (int i = 0; i < powersStates.bAvailable.Num(); i++)
	{
		powersStates.bAvailable[i] = false;

		if (powersStates.bUnlocked[i]) {
			SetPowerColor(i, 0.15f);
		}
	}

	// Reset powers.
	*powersStates.position = FVector::ZeroVector;
	*powersStates.rotation = FRotator::ZeroRotator;
	*powersStates.scale = FVector::ZeroVector;
}
