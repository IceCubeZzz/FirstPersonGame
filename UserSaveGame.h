// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UserSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class DREAMREALM_API UUserSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	float PlayerSensitivity = 1.0f;
};
