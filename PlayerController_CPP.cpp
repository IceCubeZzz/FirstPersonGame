// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "PlayerController_CPP.h"
#include "DreamRealmGameInstance.h"

APlayerController_CPP::APlayerController_CPP()
{
	bAutoManageActiveCameraTarget = true;
}

void APlayerController_CPP::AddPitchInput(float Val)
{
	if(DreamRealmGameInstance->GetUserSave())
	{
		Super::AddPitchInput(Val * DreamRealmGameInstance->GetUserSave()->PlayerSensitivity);
	}
	else
	{
		Super::AddPitchInput(Val);
	}
}

void APlayerController_CPP::AddYawInput(float Val)
{
	if (DreamRealmGameInstance->GetUserSave())
	{
		Super::AddYawInput(Val * DreamRealmGameInstance->GetUserSave()->PlayerSensitivity);
	}
	else
	{ 
		Super::AddYawInput(Val);
	}
}

void APlayerController_CPP::BeginPlay()
{
	Super::BeginPlay();

	DreamRealmGameInstance = Cast<UDreamRealmGameInstance>(GetGameInstance());
}
