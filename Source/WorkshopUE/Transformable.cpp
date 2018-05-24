// Fill out your copyright notice in the Description page of Project Settings.

#include "Transformable.h"

// Sets default values
ATransformable::ATransformable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	timeToChange = 1;
	timerLoc = 0.0;
	timerRot = 0.0;
	timerScale = 0.0;

	isModifyingLoc = false;
	isModifyingRot = false;
	isModifyingScale = false;

	initialLocation = FVector::ZeroVector;
	initialRotation = FRotator::ZeroRotator;
	initialScale = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void ATransformable::BeginPlay()
{
	Super::BeginPlay();

	root = GetRootComponent();

	baseLocation = root->RelativeLocation;
	baseRotation = root->RelativeRotation;
	baseScale = root->RelativeScale3D;

	Setup();
}

void ATransformable::Setup() {
	// Setup transformation vectors
	oldLocation = std::make_shared<FVector>(initialLocation);
	actualLocation = std::make_shared<FVector>(initialLocation);
	newLocation = std::make_shared<FVector>(initialLocation);

	oldRotation = std::make_shared<FRotator>(initialRotation);
	actualRotation = std::make_shared<FRotator>(initialRotation);
	newRotation = std::make_shared<FRotator>(initialRotation);

	oldScale = std::make_shared<FVector>(initialScale);
	actualScale = std::make_shared<FVector>(initialScale);
	newScale = std::make_shared<FVector>(initialScale);

	// Setup transformable
	bPower1 = isModifyingLoc = !newLocation->Equals(FVector::ZeroVector);
	bPower2 = isModifyingRot = !newRotation->Equals(FRotator::ZeroRotator);
	bPower3 = isModifyingScale = !newScale->Equals(FVector::ZeroVector);

	ChangeColor();
}

void ATransformable::Reset() {
	Setup();

	timerLoc = 0.0f;
	timerRot = 0.0f;
	timerScale = 0.0f;
	isModifyingLoc = isModifyingRot = isModifyingScale = false;
	ApplyLocationChange(0.0f);
	ApplyRotationChange(0.0f);
	ApplyScaleChange(0.0f);
}

// Called every frame
void ATransformable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isModifyingLoc) {
		timerLoc += DeltaTime;
		if (timerLoc >= timeToChange || timeToChange == 0.0) {
			ApplyLocationChange(1);
			isModifyingLoc = false;
			*oldLocation = *newLocation;
		}
		else if (timeToChange > 0.0) {
			float alpha = timerLoc / timeToChange;
			ApplyLocationChange(alpha);
		}
	}

	if (isModifyingRot) {
		timerRot += DeltaTime;
		if (timerRot >= timeToChange || timeToChange == 0.0) {
			ApplyRotationChange(1);
			isModifyingRot = false;
			*oldRotation = *newRotation;
		}
		else if (timeToChange > 0.0) {
			float alpha = timerRot / timeToChange;
			ApplyRotationChange(alpha);
		}
	}

	if (isModifyingScale) {
		timerScale += DeltaTime;
		if (timerScale >= timeToChange || timeToChange == 0.0) {
			ApplyScaleChange(1);
			isModifyingScale = false;
			*oldScale = *newScale;
		}
		else if (timeToChange > 0.0) {
			float alpha = timerScale / timeToChange;
			ApplyScaleChange(alpha);
		}
	}
}

void ATransformable::TransformEffect(int powerIndex)
{
	switch (powerIndex)
	{
		case 0:
			if (isModifyingLoc) {
				*oldLocation = *actualLocation;
			}
			*newLocation += FVector(300.0, 0.0, 0.0);
			timerLoc = 0.0;
			isModifyingLoc = true;
			ChangeColor(FVector(1.0, 0.0, 0.0));
			break;

		case 1:
			if (isModifyingRot) {
				*oldRotation = *actualRotation;
			}
			*newRotation += FRotator(0.0, 0.0, 45.0);
			timerRot = 0.0;
			isModifyingRot = true;
			ChangeColor(FVector(0.0, 1.0, 0.0));
			break;

		case 2:
			if (isModifyingScale) {
				*oldScale = *actualScale;
			}
			*newScale += FVector(1.0, 1.0, 1.0);
			timerScale = 0.0;
			isModifyingScale = true;
			ChangeColor(FVector(0.0, 0.0, 1.0));
			break;

		default: break;
	}
}

void ATransformable::ApplyLocationChange(float alpha)
{
	*actualLocation = FMath::Lerp(*oldLocation, *newLocation, alpha);
	root->SetRelativeLocation(baseLocation + *actualLocation);
}

void ATransformable::ApplyRotationChange(float alpha)
{
	*actualRotation = FMath::Lerp(*oldRotation, *newRotation, alpha);
	root->SetRelativeRotation(FQuat(baseRotation + *actualRotation));
}

void ATransformable::ApplyScaleChange(float alpha)
{
	*actualScale = FMath::Lerp(*oldScale, *newScale, alpha);
	root->SetRelativeScale3D(baseScale + *actualScale);
}

bool ATransformable::CheckPowerPresent(int index)
{
	if (index == 0 && bPower1) {
		return true;
	}

	if (index == 1 && bPower2) {
		return true;
	}

	if (index == 2 && bPower3) {
		return true;
	}

	return false;
}

void ATransformable::PutPowerEffect(int index, UGunComponent * gunComponent)
{
	bool powerTmp = false;

	switch (index)
	{
	case 0:
		if (isModifyingLoc) {
			*oldLocation = *actualLocation;
		}

		newLocation.swap(gunComponent->powersStates.position);

		timerLoc = 0.0;
		isModifyingLoc = true;

		powerTmp = bPower1;
		bPower1 = !newLocation->Equals(FVector::ZeroVector);

		if (bPower1 && powerTmp) {
			gunComponent->SetPowerAvailable(index);
		}
		break;

	case 1:
		if (isModifyingRot) {
			*oldRotation = *actualRotation;
		}
		newRotation.swap(gunComponent->powersStates.rotation);
		timerRot = 0.0;
		isModifyingRot = true;

		powerTmp = bPower2;
		bPower2 = !newRotation->Equals(FRotator::ZeroRotator);

		if (bPower2 && powerTmp) {
			gunComponent->SetPowerAvailable(index);
		}
		break;

	case 2:
		if (isModifyingScale) {
			*oldScale = *actualScale;
		}
		newScale.swap(gunComponent->powersStates.scale);
		timerScale = 0.0;
		isModifyingScale = true;

		powerTmp = bPower3;
		bPower3 = !newScale->Equals(FVector::ZeroVector);

		if (bPower3 && powerTmp) {
			gunComponent->SetPowerAvailable(index);
		}
		break;

	default: break;
	}

	ChangeColor();
}

void ATransformable::ChangeColor() {

	float r = 0.0f, g = 0.0f, b = 0.0f; // Black

	if (bPower1) {
		if (bPower2) {
			if (bPower3) {
				r = g = b = 1.0f; // White
			}
			else {
				r = 1.0f; // Orange
				g = 0.5f;
			}
		}
		else {
			if (bPower3) {
				r = 0.5f; // Turquoise
				b = 1.0f;
			}
			else {
				r = 1.0f; // Red
			}
		}
	}
	else {
		if (bPower2) {
			if (bPower3) {
				g = 1.0f; // Purple
				b = 0.5;
			}
			else {
				g = 1.0f; // Green
			}
		}
		else {
			if (bPower3) {
				b = 1.0f; // Blue
			}
		}
	}

	ChangeColor(FVector(r, g, b));
}
