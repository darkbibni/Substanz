// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include <memory>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunComponent.h"
#include "Transformable.generated.h"

UCLASS()
class WORKSHOPUE_API ATransformable : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = Gameplay)
	float timeToChange;

	bool isModifyingLoc;
	FVector baseLocation;
	UPROPERTY(EditAnywhere, Category = Gameplay)
	FVector initialLocation;
	std::shared_ptr<FVector> oldLocation;
	std::shared_ptr<FVector> actualLocation;
	std::shared_ptr<FVector> newLocation;

	bool isModifyingRot;
	FRotator baseRotation;
	UPROPERTY(EditAnywhere, Category = Gameplay)
	FRotator initialRotation;
	std::shared_ptr<FRotator> oldRotation;
	std::shared_ptr<FRotator> actualRotation;
	std::shared_ptr<FRotator> newRotation;

	bool isModifyingScale;
	FVector baseScale;
	UPROPERTY(EditAnywhere, Category = Gameplay)
	FVector initialScale;
	std::shared_ptr<FVector> oldScale;
	std::shared_ptr<FVector> actualScale;
	std::shared_ptr<FVector> newScale;

private:
	class USceneComponent *root;

	float timerLoc;
	float timerRot;
	float timerScale;

public:
	// Sets default values for this actor's properties
	ATransformable();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, category = "CppFunctions")
	void ChangeColor(FVector color);

	void TransformEffect(int powerIndex);

	/* Check if the specified power is present on this transformable */
	bool CheckPowerPresent(int index);

	/* Put effect on transformable and swap if same power already exist. */
	void PutPowerEffect(int index, UGunComponent* gunComponent);

	void Reset();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void ApplyLocationChange(float alpha);
	void ApplyRotationChange(float alpha);
	void ApplyScaleChange(float alpha);

	bool bPower1, bPower2, bPower3;

	void Setup();

	void ChangeColor();
};
