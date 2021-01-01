// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController_CPP.generated.h"

/**
 * 
 */
UCLASS()
class DREAMREALM_API ABaseAIController_CPP : public AAIController
{
	GENERATED_BODY()
	
public:
	ABaseAIController_CPP();

	virtual void OnPossess(APawn* InPawn) override;

	uint8 EnemyKeyID;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	 * Default implementation of this event by Epic Games, for unknown reasons,
	 * sets AI pitch to zero when the AI is looking at a point instead of a pawn.
	 * This is disabled in this override
	 */
	void UpdateControlRotation(float DeltaTime, bool bUpdatePawn) override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	// Stops running AI logic
	void OnDeath();

protected:
	UPROPERTY(BlueprintReadOnly)
	class UBlackboardComponent* BlackboardComp;

	UPROPERTY(BlueprintReadOnly)
	class UBehaviorTreeComponent* BehaviorComp;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	FVector AimOffset = FVector(0, 0, 75);

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	bool ShouldRotateTowardsTarget = true;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	AActor* LocalTarget;
};
