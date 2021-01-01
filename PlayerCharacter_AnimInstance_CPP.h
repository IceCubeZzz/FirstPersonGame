// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnumWeaponType.h"
#include "PlayerCharacter_AnimInstance_CPP.generated.h"

/**
 * 
 */
UCLASS()
class DREAMREALM_API UPlayerCharacter_AnimInstance_CPP : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EWeaponType> SelectedWeapon = EWeaponType::SilencedPistol;

public:
	void SetSelectedWeapon(EWeaponType WeaponType);
};
