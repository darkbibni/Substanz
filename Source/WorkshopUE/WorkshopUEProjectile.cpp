// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "WorkshopUEProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

AWorkshopUEProjectile::AWorkshopUEProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(1.5f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AWorkshopUEProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 5000.f;
	ProjectileMovement->MaxSpeed = 5000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// Die after 2 seconds by default
	lifespan = 2.0f; // customizable in editor.

	InitialLifeSpan = lifespan;

	bHasHitTransformable = false;
}

void AWorkshopUEProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		ATransformable* t = Cast<ATransformable>(OtherActor);

		if (t != NULL) {
			player->AddTransformable(t);

			t->PutPowerEffect(gunComponent->currentPower, gunComponent);

			bHasHitTransformable = true; // Prevent to keep the power in the gun.

			Destroy();
		}
	}
}

void AWorkshopUEProjectile::Destroyed() {

	if (!bHasHitTransformable) {
		if (gunComponent) {
			gunComponent->SetPowerAvailable(powerIndex);
		}
	}

	Super::Destroyed();
}