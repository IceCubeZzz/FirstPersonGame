// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

// Need includes between these comments when making a BTTask
#include "BaseAIController_CPP.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
//
#include "BaseEnemy_CPP.h"



ABaseAIController_CPP::ABaseAIController_CPP()
{
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));

	PrimaryActorTick.bCanEverTick = true;
}


void ABaseAIController_CPP::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseAIController_CPP::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABaseEnemy_CPP* BaseEnemy = Cast<ABaseEnemy_CPP>(InPawn);

	if (BaseEnemy && (BaseEnemy->BehaviorTree))
	{
		BlackboardComp->InitializeBlackboard(*(BaseEnemy->BehaviorTree->BlackboardAsset));

		EnemyKeyID = BlackboardComp->GetKeyID("Target");

		BehaviorComp->StartTree(*(BaseEnemy->BehaviorTree));
	}

	// Set focus to nothing
	SetFocus(nullptr);
}

void ABaseAIController_CPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ShouldRotateTowardsTarget)
	{
		if(!LocalTarget)
		{
			// Obtain reference to the target (the player)
			LocalTarget = Cast<AActor>(BlackboardComp->GetValueAsObject("Target"));
		}
		if (LocalTarget)
		{
			// Rotate towards player 
			SetFocalPoint(LocalTarget->GetActorLocation() + AimOffset);
		}
	}
}

void ABaseAIController_CPP::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		FRotator NewControlRotation = GetControlRotation();

		// Look toward focus
		const FVector FocalPoint = GetFocalPoint();
		if (FAISystem::IsValidLocation(FocalPoint))
		{
			NewControlRotation = (FocalPoint - MyPawn->GetPawnViewLocation()).Rotation();
		}
		else if (bSetControlRotationFromPawnOrientation)
		{
			NewControlRotation = MyPawn->GetActorRotation();
		}

		// Don't pitch view unless looking at another pawn
		/*if (NewControlRotation.Pitch != 0 && Cast<APawn>(GetFocusActor()) == nullptr)
		{
			NewControlRotation.Pitch = 0.f;
		}*/

		SetControlRotation(NewControlRotation);

		if (bUpdatePawn)
		{
			const FRotator CurrentPawnRotation = MyPawn->GetActorRotation();

			if (CurrentPawnRotation.Equals(NewControlRotation, 1e-3f) == false)
			{
				MyPawn->FaceRotation(NewControlRotation, DeltaTime);
			}
		}
	}
}

