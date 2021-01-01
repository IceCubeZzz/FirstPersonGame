// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InterfaceUsable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInterfaceUsable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAMREALM_API IInterfaceUsable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int Use();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FString UseText();
};
