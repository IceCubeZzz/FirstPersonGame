// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "Engine/UserDefinedEnum.h"
#include "EnumDirection.generated.h"

/**
 * 
 */
/*UCLASS()
class DREAMREALM_API UEnumDirection : public UUserDefinedEnum
{
	GENERATED_BODY()
	
};*/

UENUM(BlueprintType)
enum EDirection
{
	Front UMETA(DisplayName = "Forward "),
	Back UMETA(DisplayName = "Backward "),
	Right UMETA(DisplayName = "Rightward "),
	Left UMETA(DisplayName = "Leftward "),
};