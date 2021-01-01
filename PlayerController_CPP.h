// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController_CPP.generated.h"

/**
 * 
 */
UCLASS()
class DREAMREALM_API APlayerController_CPP : public APlayerController
{
	GENERATED_BODY()
	
public:
	APlayerController_CPP();

	/**
	 * Overided so that the input to the parent method can be multiplied by the 
	 * player's sensitivity value
	 */
	virtual void AddPitchInput(float Val) override;

	/**
	 * Overided so that the input to the parent method can be multiplied by the
	 * player's sensitivity value
	 */
	virtual void AddYawInput(float Val) override;

protected:
	virtual void BeginPlay() override;

	// Pointer to the Game Instance. Set in BeginPlay 	
	class UDreamRealmGameInstance* DreamRealmGameInstance;
};
