// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "BTTask_FindRandomPointInNav.h"
#include "AIController.h" 
#include "NavigationSystem.h" 
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UBTTask_FindRandomPointInNav::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FNavLocation RandomPoint;

	// First, check if navigation system cast succeeds
	UNavigationSystemV1* NavSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
	if(NavSystem)
	{
		// Generate random reachable point 
		if (NavSystem->GetRandomReachablePointInRadius(OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation(), Radius, RandomPoint))
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), RandomPoint.Location);
			return EBTNodeResult::Succeeded;
		}
		else
			return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Failed;
}