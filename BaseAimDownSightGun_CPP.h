// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseGun.h"
#include "BaseAimDownSightGun_CPP.generated.h"

UCLASS()
class DREAMREALM_API ABaseAimDownSightGun_CPP : public ABaseGun
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseAimDownSightGun_CPP();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// "Special ability" for guns of this class is the ability to aim down sights
	virtual void EnableSpecialAbility() override;
	virtual void DisableSpecialAbility() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
