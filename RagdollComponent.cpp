// Fill out your copyright notice in the Description page of Project Settings.

#include "RagdollComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMeshSocket.h"

#include "Engine/Engine.h"


// Sets default values for this component's properties
URagdollComponent::URagdollComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void URagdollComponent::BeginPlay()
{
	Super::BeginPlay();

}


// Called every frame
void URagdollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*if (IsRagdoll)
	{
		if (SkeletalMeshPtr && CapsuleComponentPtr)
		{
			CapsuleComponentPtr->SetWorldLocation(FMath::VInterpTo(CapsuleComponentPtr->GetComponentLocation(), CalculateMeshLocation(), DeltaTime, 3));
		}
	}*/
}

void URagdollComponent::InitRagdollComponent(USkeletalMeshComponent* SkeletalMesh, UCharacterMovementComponent* MovementComponent, UCapsuleComponent* CapsuleComponent)
{
	SkeletalMeshPtr = SkeletalMesh;
	MovementComponentPtr = MovementComponent;
	CapsuleComponentPtr = CapsuleComponent;
}

void URagdollComponent::EnableRagdollHumanoid()
{
	IsRagdoll = true;

	MovementComponentPtr->DisableMovement();
	SkeletalMeshPtr->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SkeletalMeshPtr->SetSimulatePhysics(true);
	//SkeletalMeshPtr->SetAllBodiesBelowSimulatePhysics("pelvis", true, true);
	SkeletalMeshPtr->SetAllBodiesBelowPhysicsBlendWeight("pelvis", 1.0, false, true);
	CapsuleComponentPtr->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void URagdollComponent::DisableRagdollHumanoid()
{
	IsRagdoll = false;

	CapsuleComponentPtr->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MoveCapsuleToMeshOnDisableRagdoll();
}

void URagdollComponent::MoveCapsuleToMeshOnDisableRagdoll()
{
	MoveCapsuleToMesh();
}

// Separated into another function in case want to implement an async linetrace instead of a sync linetrace (for optimization purposes)
void URagdollComponent::MoveCapsuleToMesh()
{
	FVector MoveCapsuleTo = CalculateMeshLocation();

	//Possible interp capsule location in future
	//FMath::VInterpTo( MoveCapsuleTo), CapsuleComponent->GetComponentLocation(), GetWorld()->GetDeltaSeconds, 1);
}

bool URagdollComponent::IsPlayerFacingUp()
{
	FRotator PelvisRotation = SkeletalMeshPtr->GetSocketRotation("pelvis");

	//UE_LOG(LogClass, Log, TEXT("Pelvis rot: %f"), PelvisRotation.GetComponentForAxis(EAxis::Z));

	if (UKismetMathLibrary::GetRightVector(PelvisRotation).Z >= 0)
		return true;
	else
		return false;
}

void URagdollComponent::SetupGetupOrientation()
{
	FVector NeckLocation = SkeletalMeshPtr->GetSocketLocation("neck_01");
	FVector PelvisLocation = SkeletalMeshPtr->GetSocketLocation("pelvis");

	bool FacingUp = IsPlayerFacingUp();

	FVector OrientVector;

	//If player facing up, we want to subtract neck location from the pelvis location
	//If player facing down, we do this the other way around

	if (FacingUp)
	{
		OrientVector = PelvisLocation - NeckLocation;
	}
	else
	{
		OrientVector = NeckLocation - PelvisLocation;
	}

	FVector MeshLocation = CalculateMeshLocation();
	FRotator MeshRotation = UKismetMathLibrary::MakeRotFromZX(FVector(0, 0, 1), OrientVector);
	FTransform Transform = FTransform(MeshRotation, MeshLocation);

	GetOwner()->SetActorTransform(Transform);
}

UAnimMontage* URagdollComponent::SetupGetupMontage(UAnimMontage* MontageUp, UAnimMontage* MontageDown)
{
	if (IsPlayerFacingUp())
	{
		return MontageUp;
	}
	else
		return MontageDown;
}

FVector URagdollComponent::CalculateMeshLocation()
{
	//FVector PelvisLocation = SkeletalMeshPtr->GetBoneLocation("pelvis", EBoneSpaces::WorldSpace);
	FVector PelvisLocation = SkeletalMeshPtr->GetSocketLocation("pelvis");

	FHitResult OutHit;
	FVector End = (PelvisLocation - FVector(0.0f, 0.0f, -100.0f));
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(GetOwner());

	FVector MeshTransform;

	if (GetWorld()->LineTraceSingleByChannel(OutHit, PelvisLocation, End, ECollisionChannel::ECC_Visibility, TraceParams))
	{
		MeshTransform = OutHit.Location + FVector(0, 0, PelvisHeight);
	}
	else
	{
		MeshTransform = PelvisLocation + FVector(0, 0, PelvisHeight);
	}

	return MeshTransform;
}

bool URagdollComponent::GetIsRagdoll()
{
	return IsRagdoll;
}

