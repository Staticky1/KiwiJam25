// Copyright Epic Games, Inc. All Rights Reserved.

#include "KiwiJam2025GameMode.h"
#include "KiwiJam2025Character.h"
#include "UObject/ConstructorHelpers.h"

AKiwiJam2025GameMode::AKiwiJam2025GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
