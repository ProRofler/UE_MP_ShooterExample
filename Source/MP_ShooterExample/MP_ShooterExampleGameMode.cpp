// Copyright Epic Games, Inc. All Rights Reserved.

#include "MP_ShooterExampleGameMode.h"
#include "MP_ShooterExampleCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMP_ShooterExampleGameMode::AMP_ShooterExampleGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
