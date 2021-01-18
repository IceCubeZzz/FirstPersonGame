// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#include "DreamRealmGameInstance.h"
#include "Weapon_CPP.h"
#include "BaseGun_CPP.h"

UDreamRealmGameInstance::UDreamRealmGameInstance()
{
	
}

void UDreamRealmGameInstance::Init()
{
	Super::Init();

	// Check if save game file exists
	USaveGame* UserSaveFile = UGameplayStatics::LoadGameFromSlot(UserSaveSlotName, UserSaveSlotIndex);
	if (UserSaveFile)
	{
		// If file exists, cast to correct save game type and save a pointer ref
		UserSave = Cast<UUserSaveGame>(UserSaveFile);
	}
	else
	{
		// If no file exists, create one
		UserSave = Cast<UUserSaveGame>(UGameplayStatics::CreateSaveGameObject(UUserSaveGame::StaticClass()));
		UGameplayStatics::SaveGameToSlot(UserSave, UserSaveSlotName, UserSaveSlotIndex);
	}

	PlayerUnlockedWeapons.Add(EWeaponType::SilencedPistol, true);
	PlayerUnlockedWeapons.Add(EWeaponType::BoltActionRifle, true);
	PlayerUnlockedWeapons.Add(EWeaponType::SubmachineGun, true);
}

UUserSaveGame* UDreamRealmGameInstance::GetUserSave()
{
	return UserSave;
}
