// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerSettings.generated.h"

/**
 *
 */
UCLASS()
class DREAMREALM_API UPlayerSettings : public USaveGame
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = Basic)
    FString PlayerName;

    /* Settings */

    //Volume Settings (Range from 1-100)
    UPROPERTY(BlueprintReadWrite, Category = Basic)
    float MasterVolume;

    UPROPERTY(BlueprintReadWrite, Category = Basic)
    float EffectsVolume;

    UPROPERTY(BlueprintReadWrite, Category = Basic)
    float MusicVolume;

    UPROPERTY(BlueprintReadWrite, Category = Basic)
    float VOIPVolume;

    //Graphics Settings (Currently unused)
    UPROPERTY(BlueprintReadWrite, Category = Basic)
    float rViewDistanceScale;

    UPlayerSettings();
};
