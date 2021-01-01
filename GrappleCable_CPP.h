// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CableActor.h"
#include "GrappleCable_CPP.generated.h"

/**
 * 
 */
UCLASS()
class DREAMREALM_API AGrappleCable_CPP : public ACableActor
{
	GENERATED_BODY()

public:
	AGrappleCable_CPP();

protected:
	virtual void BeginPlay() override;
};
