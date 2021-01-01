// Copyright Epic Games, Inc. All Rights Reserved.

#include "DreamRealmGameMode.h"
#include "DreamRealmHUD.h"
#include "DreamRealmCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADreamRealmGameMode::ADreamRealmGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControlerClassFinder(TEXT("/Game/_PlayerCharacter/DefaultPlayerController"));
	PlayerControllerClass = PlayerControlerClassFinder.Class;

	// use our custom HUD class
	HUDClass = ADreamRealmHUD::StaticClass();
}
