// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseProjectile.h"
#include "LaserProjectile.generated.h"

/**
 * 
 */
UCLASS()
class DREAMREALM_API ALaserProjectile : public ABaseProjectile
{
	GENERATED_BODY()
public:
	ALaserProjectile();

protected:
	virtual void BeginPlay() override;
};
