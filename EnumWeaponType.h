// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "Engine/UserDefinedEnum.h"
#include "EnumWeaponType.generated.h"


/*UCLASS()
class DREAMREALM_API UEnumWeaponType : public UUserDefinedEnum
{
	GENERATED_BODY()
	
};*/

UENUM(BlueprintType)
enum EWeaponType
{
	AIWeapon UMETA(DisplayName = "AIWeapon"),
	SilencedPistol UMETA(DisplayName = "Silenced Pistol"),
	BoltActionRifle UMETA(DisplayName = "Bolt-Action Rifle"),
	SubmachineGun UMETA(DisplayName = "Submachine Gun"),
};

