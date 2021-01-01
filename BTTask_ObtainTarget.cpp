// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "BTTask_ObtainTarget.h"
#include "BaseAIController_CPP.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UBTTask_ObtainTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (GetWorld()->GetFirstPlayerController()->GetPawn())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(GetSelectedBlackboardKey(), GetWorld()->GetFirstPlayerController()->GetPawn());
		return EBTNodeResult::Succeeded;
	}
	else
	{
		return EBTNodeResult::Failed;
	}
}
