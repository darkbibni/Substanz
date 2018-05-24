// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "WorkshopUECharacter.h"
#include "WorkshopUEProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AWorkshopUECharacter

AWorkshopUECharacter::AWorkshopUECharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	rayLength = 4000.0f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	{
		Arms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
		Arms->SetOnlyOwnerSee(true);
		Arms->SetupAttachment(FirstPersonCameraComponent);
		Arms->bCastDynamicShadow = false;
		Arms->CastShadow = false;

		Arms->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
		Arms->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);
	}

	// Create mesh gun canon
	{
		FP_GunCanon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FP_TransformGun"));
		FP_GunCanon->SetOnlyOwnerSee(false);			// only the owning player will see this mesh
		FP_GunCanon->bCastDynamicShadow = false;
		FP_GunCanon->CastShadow = false;
		FP_GunCanon->SetupAttachment(RootComponent);



		// Transform
		FP_GunCanon->RelativeRotation = FRotator(0.0f, 180.f, -90.0f);
		FP_GunCanon->RelativeLocation = FVector(4.0f, 50.0f, 2.0f);
		FP_GunCanon->RelativeScale3D = FVector(0.3f, 0.3f, 0.3f);
	}

	// Create mesh gun tubes
	{
		FP_GunTubes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FP_GunTubes"));
		FP_GunTubes->SetOnlyOwnerSee(true);
		FP_GunTubes->bCastDynamicShadow = false;
		FP_GunTubes->CastShadow = false;
		FP_GunTubes->SetupAttachment(FP_GunCanon);

		// Transform
		FP_GunTubes->RelativeRotation = FRotator(0.0f, 60.0f, -180.0f);
		FP_GunTubes->RelativeLocation = FVector(0.0f, 0.0f, 80.0f);
	}

	// Setup shoot origin
	{
		shootOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
		shootOrigin->SetupAttachment(FP_GunCanon);

		// Transform
		shootOrigin->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	}

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
}

void AWorkshopUECharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	lastCheckpoint = GetActorLocation();

	gunComponent = FindComponentByClass<UGunComponent>();
	gunComponent->gunTubes = FP_GunTubes;

	DisplayGun(false);

	FP_GunCanon->AttachToComponent(Arms, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.

	Arms->SetHiddenInGame(false, true);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AWorkshopUECharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind actions event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AWorkshopUECharacter::OnFire);
	PlayerInputComponent->BindAction("Absorb", IE_Pressed, this, &AWorkshopUECharacter::OnAbsorb);

	PlayerInputComponent->BindAction("Power1", IE_Pressed, this, &AWorkshopUECharacter::SwitchToPower1);
	PlayerInputComponent->BindAction("Power2", IE_Pressed, this, &AWorkshopUECharacter::SwitchToPower2);
	PlayerInputComponent->BindAction("Power3", IE_Pressed, this, &AWorkshopUECharacter::SwitchToPower3);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AWorkshopUECharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWorkshopUECharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AWorkshopUECharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AWorkshopUECharacter::LookUpAtRate);

	// Change power
	PlayerInputComponent->BindAxis("ChangePower", this, &AWorkshopUECharacter::ChangePower);
}

void AWorkshopUECharacter::OnFire()
{
	if (!gunComponent->IsEnable()) {
		return;
	}

	// Check projectile classe
	if (gunComponent->ProjectileClasses[gunComponent->currentPower] == NULL)
	{
		return; // TODO debug for all ?
	}

	// Check world
	UWorld* const World = GetWorld();
	if (World == NULL)
	{
		return;
	}

	// Check if gun can fire his power.
	if (gunComponent->TryToUsePower()) {
		// Spawn projectile
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = shootOrigin->GetComponentToWorld().GetLocation();

			// Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Spawn the projectile at the muzzle
			AWorkshopUEProjectile* projectile = World->SpawnActor<AWorkshopUEProjectile>(gunComponent->ProjectileClasses[gunComponent->currentPower], SpawnLocation, SpawnRotation, ActorSpawnParams);
			
			if (projectile) {
				projectile->gunComponent = gunComponent;
				projectile->player = this;
			}
		}

		// try and play the sound if specified
		if (FireSound != NULL)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		// try and play a firing animation if specified
		if (FireAnimation != NULL)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = Arms->GetAnimInstance();
			if (AnimInstance != NULL)
			{
				AnimInstance->Montage_Play(FireAnimation, 1.f);
			}
		}
	}
}

void AWorkshopUECharacter::OnAbsorb()
{
	if (!gunComponent->IsEnable()) {
		return;
	}

	// Check if can absorb the power on targeted transformable

	// Raycast
	if (Controller && Controller->IsLocalPlayerController()) {

		const FVector StartTrace = shootOrigin->GetComponentToWorld().GetLocation(); // trace start is the camera location
		const FVector Direction = FirstPersonCameraComponent->GetForwardVector();
		const FVector EndTrace = StartTrace + Direction * rayLength; // and trace end is the camera location + an offset in the direction you are looking, the 200 is the distance at wich it checks

		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECollisionChannel::ECC_Visibility, TraceParams)) {

			ATransformable* t = Cast<ATransformable>(Hit.GetActor());

			// Minimal Feedback !
			UKismetSystemLibrary::DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, 0.2f, 2.0f);

			// try and play a firing animation if specified
			if (FireAnimation != NULL)
			{
				// Get the animation object for the arms mesh
				UAnimInstance* AnimInstance = Arms->GetAnimInstance();
				if (AnimInstance != NULL)
				{
					AnimInstance->Montage_Play(FireAnimation, 1.f);
				}
			}

			if (t != NULL) {
				if (t->CheckPowerPresent(gunComponent->currentPower)) {

					t->PutPowerEffect(gunComponent->currentPower, gunComponent);

					gunComponent->AbsorbPower();

					AddTransformable(t);

					// try and play the sound if specified
					if (AbsorbSound != NULL)
					{
						UGameplayStatics::PlaySoundAtLocation(this, AbsorbSound, GetActorLocation());
					}
				}
			}
		}
	}
}

void AWorkshopUECharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AWorkshopUECharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = false;
}

void AWorkshopUECharacter::ResetTransformables()
{
	for (int i = 0; i < transformablesUsed.Num(); i++)
	{
		transformablesUsed[i]->Reset();
	}

	transformablesUsed.Reset();
}

void AWorkshopUECharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AWorkshopUECharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AWorkshopUECharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AWorkshopUECharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AWorkshopUECharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AWorkshopUECharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AWorkshopUECharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AWorkshopUECharacter::TouchUpdate);
		return true;
	}
	
	return false;
}


//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AWorkshopUECharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AWorkshopUECharacter::SwitchToPower1()
{
	gunComponent->SwitchToPower(0);
}

void AWorkshopUECharacter::SwitchToPower2()
{
	gunComponent->SwitchToPower(1);
}

void AWorkshopUECharacter::SwitchToPower3()
{
	gunComponent->SwitchToPower(2);
}

void AWorkshopUECharacter::ChangePower(float value)
{
	if (value < 0) {
		gunComponent->PreviousPower();
	}
	else if(value > 0) {
		gunComponent->NextPower();
	}
}

void AWorkshopUECharacter::DisplayGun(bool displayed)
{
	Arms->SetOwnerNoSee(!displayed);
	FP_GunCanon->SetOwnerNoSee(!displayed);
	FP_GunTubes->SetOwnerNoSee(!displayed);
}

void AWorkshopUECharacter::EquipGun()
{
	DisplayGun(true);

	gunComponent->EquipGun();
}

void AWorkshopUECharacter::UnlockNewPower(int index)
{
	gunComponent->UnlockPower(index);
}

void AWorkshopUECharacter::KillPlayer() {
	gunComponent->bEquipped = false;

	FirstPersonCameraComponent->PostProcessSettings.SceneColorTint = FColor::Orange;

	FLinearColor::LerpUsingHSV(FColor::White, FColor::Red, 0.5);
}

void AWorkshopUECharacter::SetNewCheckpoint(FVector newCheckpoint) {

	lastCheckpoint = newCheckpoint;
}

void AWorkshopUECharacter::TeleportToLastCheckpoint() {
	
	// Reset gun powers and transformable affected.
	PassThroughBarrer();

	FirstPersonCameraComponent->PostProcessSettings.SceneColorTint = FColor::White;
	GetRootComponent()->SetRelativeLocation(lastCheckpoint);
	gunComponent->bEquipped = true;
}

void AWorkshopUECharacter::PassThroughBarrer()
{
	gunComponent->bEquipped = false;

	// Reset transformables affected.
	ResetTransformables();

	// Reset powers of the gun.
	gunComponent->ResetPowers();
}

void AWorkshopUECharacter::ExitBarrer() {
	gunComponent->bEquipped = true;
}

void AWorkshopUECharacter::AddTransformable(ATransformable* transformable)
{
	transformablesUsed.AddUnique(transformable);
}