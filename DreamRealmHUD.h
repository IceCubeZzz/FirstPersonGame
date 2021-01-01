// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DreamRealmHUD.generated.h"

UCLASS()
class ADreamRealmHUD : public AHUD
{
	GENERATED_BODY()

public:
	ADreamRealmHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

