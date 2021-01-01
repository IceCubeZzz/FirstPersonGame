// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "BTTask_FindRandomStrafePoint.h"
#include "AIController.h" 
#include "NavigationSystem.h" 
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
//#include "DrawDebugHelpers.h"

EBTNodeResult::Type UBTTask_FindRandomStrafePoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Random strafe point generated in following do-while loop
	FVector RandomStrafePoint;
	// The projection of the random strafe point to the navmesh
	FNavLocation RandomStrafePointProjected;

	// First, check if navigation system cast succeeds
	UNavigationSystemV1* NavSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
	if (NavSystem)
	{
		/**
		 * Variable IterationCount is used to prevent an excessive amount of iterations of the following do-while loop
		 * in case no generated point can be projected to the navmesh
		 */
		int IterationCount = 0;
		do
		{
			// Generate random point to strafe to along actor's right vector
			float Rand = FMath::RandRange(-MaxStrafeDistance, MaxStrafeDistance);
			FVector StrafeOffset = OwnerComp.GetAIOwner()->GetPawn()->GetActorRightVector() * Rand;

			RandomStrafePoint = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation() + StrafeOffset;

			++IterationCount;

			/**
			 * While the generated point is not valid (cannot be projected to nav mesh) and the iteration count is below threshold,
			 * repeat the loop
			 */ 

		}while(!(NavSystem->ProjectPointToNavigation(RandomStrafePoint, RandomStrafePointProjected)) && IterationCount < 1000);

		OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), RandomStrafePointProjected.Location);

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}