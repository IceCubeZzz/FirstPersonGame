// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UserSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "EnumWeaponType.h"
#include "DreamRealmGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class DREAMREALM_API UDreamRealmGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UDreamRealmGameInstance();

	UPROPERTY(BlueprintReadWrite)
	FString UserSaveSlotName = "UserSaveSlot";
	UPROPERTY(BlueprintReadWrite)
	int UserSaveSlotIndex = 0;

	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite)
	// The weapons the player has selected for this level
	TArray<TSubclassOf<AWeapon_CPP>> PlayerSelectedWeapons;

	UFUNCTION(BlueprintCallable)
	UUserSaveGame* GetUserSave();

protected:
	UPROPERTY(BlueprintReadWrite, Category = "SaveGame")
	UUserSaveGame* UserSave;

	UPROPERTY(BlueprintReadWrite)
	// Weapons that the player has unlocked
	TMap<TEnumAsByte<EWeaponType>, bool> PlayerUnlockedWeapons;
};
