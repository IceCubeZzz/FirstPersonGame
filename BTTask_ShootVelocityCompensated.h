// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ShootVelocityCompensated.generated.h"

/**
 * 
 */
UCLASS()
class DREAMREALM_API UBTTask_ShootVelocityCompensated : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere)
	FVector AimOffset;
};
