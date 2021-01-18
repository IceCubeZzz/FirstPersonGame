// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "BTTask_ShootTarget.h"
#include "BaseGun_CPP.h"
#include "BaseEnemy_CPP.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UBTTask_ShootTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseEnemy_CPP* Enemy = Cast<ABaseEnemy_CPP>( Cast<AAIController>(OwnerComp.GetOwner())->GetPawn() );

	if(Enemy != nullptr)
	{ 
		ABaseGun_CPP* EnemyGun = Cast<ABaseGun_CPP>(Enemy->Weapon);
		AActor* TargetRef = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("Target"));
		FVector TargetLocation = TargetRef->GetActorLocation() + AimOffset;

		if(EnemyGun != nullptr)
		{ 
			EnemyGun->SetLocationToFireTowards(TargetLocation);
			EnemyGun->Fire();
			return EBTNodeResult::Succeeded;
		}
		else
		{ 
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Cast to ABaseGun failed in BTTask_ShootTarget.cpp"));
			return EBTNodeResult::Failed;
		}
	}
	else
	{ 
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Cast to ABaseEnemy_CPP failed in BTTask_ShootTarget.cpp"));
		return EBTNodeResult::Failed;
	}
}