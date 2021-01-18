// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "BTTask_ShootVelocityCompensated.h"
#include "BaseGun_CPP.h"
#include "BaseEnemy_CPP.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UBTTask_ShootVelocityCompensated::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseEnemy_CPP* Enemy = Cast<ABaseEnemy_CPP>(Cast<AAIController>(OwnerComp.GetOwner())->GetPawn());

	if (Enemy != nullptr)
	{
		ABaseGun_CPP* EnemyGun = Cast<ABaseGun_CPP>(Enemy->Weapon);
		AActor* TargetRef = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("Target"));
		FVector TargetLocation = TargetRef->GetActorLocation() + AimOffset;

		if (EnemyGun != nullptr)
		{
			// Compensate for target's velocity by predicting a new location to fire
			float BulletTravelDistance = (TargetLocation - EnemyGun->GetActorLocation()).Size();
			float BulletTravelTime = BulletTravelDistance / EnemyGun->GetProjectileVelocity();
			FVector TargetVelocityCompensation = TargetRef->GetVelocity() * BulletTravelTime;

			// Find the location to fire at by adding the target's current location to a vector 
			// representing the distance the target will move while the bullet is traveling
			FVector PredictedLocation = TargetLocation + TargetVelocityCompensation;

			EnemyGun->SetLocationToFireTowards(PredictedLocation);
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