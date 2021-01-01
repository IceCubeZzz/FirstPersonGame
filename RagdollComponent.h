// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RagdollComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DREAMREALM_API URagdollComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URagdollComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void MoveCapsuleToMesh();

	UPROPERTY(BlueprintReadWrite)
	float PelvisHeight = 62.0f;

	class USkeletalMeshComponent* SkeletalMeshPtr;
	class UCharacterMovementComponent* MovementComponentPtr;
	class UCapsuleComponent* CapsuleComponentPtr;

	bool IsRagdoll = false;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void InitRagdollComponent(USkeletalMeshComponent* SkeletalMesh, UCharacterMovementComponent* MovementComponent, UCapsuleComponent* CapsuleComponent);

	UFUNCTION(BlueprintCallable)
	void EnableRagdollHumanoid();

	UFUNCTION(BlueprintCallable)
	bool IsPlayerFacingUp();

	UFUNCTION(BlueprintCallable)
	void MoveCapsuleToMeshOnDisableRagdoll();

	UFUNCTION(BlueprintCallable)
	void SetupGetupOrientation();

	UFUNCTION(BlueprintCallable)
	class UAnimMontage* SetupGetupMontage(UAnimMontage* MontageUp, UAnimMontage* MontageDown);

	UFUNCTION(BlueprintCallable)
	void DisableRagdollHumanoid();

	FVector CalculateMeshLocation();

	UFUNCTION(BlueprintCallable)
	bool GetIsRagdoll();
};
