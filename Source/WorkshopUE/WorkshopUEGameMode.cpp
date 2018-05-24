// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "WorkshopUEGameMode.h"
#include "WorkshopUEHUD.h"
#include "WorkshopUECharacter.h"
#include "UObject/ConstructorHelpers.h"

AWorkshopUEGameMode::AWorkshopUEGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_FpsController"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AWorkshopUEHUD::StaticClass();
}
