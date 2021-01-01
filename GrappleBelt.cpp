// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "GrappleBelt.h"
#include "BaseProjectile.h"
#include "Components/SceneComponent.h" 
#include "GrappleHookProjectile_CPP.h"
#include "GrappleCable_CPP.h"
#include "CableComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h" 

#include "DrawDebugHelpers.h"

// Sets default values
AGrappleBelt::AGrappleBelt()
{
	// Disable tick to improve performance
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootCompnent");
	CableAttachPointBelt = CreateDefaultSubobject<USceneComponent>("GrappleAttachPoint");

	SetRootComponent(SceneComponent);
}
 
// Called when the game starts or when spawned
void AGrappleBelt::BeginPlay()
{
	Super::BeginPlay();

	CableAttachPointBelt->AttachToComponent(SceneComponent, FAttachmentTransformRules::KeepRelativeTransform);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GrappleCable = GetWorld()->SpawnActor<AGrappleCable_CPP>(GrappleCableClass, CableAttachPointBelt->GetComponentLocation(),
		CableAttachPointBelt->GetComponentRotation(), SpawnParams);

	// Attach end of cable to this actor at specified attach point
	GrappleCable->CableComponent->SetAttachEndToComponent(this->CableAttachPointBelt);
	// Always reset cable component's end offset to zero after setting a new end attach
	GrappleCable->CableComponent->EndLocation = FVector::ZeroVector;
	GrappleCable->CableComponent->SetVisibility(false);
	GrappleCable->CableComponent->CableLength = DefaultCableLength;
}

void AGrappleBelt::HandleHookCooldown(float DeltaTime)
{
	if (HookCooldownTimer > 0)
	{
		HookCooldownTimer -= DeltaTime;
	}
	if (FireNoHitTimer > 0)
	{
		FireNoHitTimer -= DeltaTime;
		if (FireNoHitTimer <= 0)
		{
			FireNoHitTimer = 0;
			// Hook projectile has hit nothing. Reel back the projectile
			UnHook();
		}
	}
	if (ProjectilePhysicsDisableTimer > 0)
	{
		ProjectilePhysicsDisableTimer -= DeltaTime;
		if (ProjectilePhysicsDisableTimer <= 0)
		{
			ProjectilePhysicsDisableTimer = 0;

			// Disable collision so that grapple hook projectile can be retracted successfully 
			GrappleHookProjectile->GetProjectileMesh()->SetCollisionObjectType(IGNORE_ALL);
		}
	}
}

bool AGrappleBelt::CanFire()
{
	return (HookCooldownTimer <= 0 && !FiredHook && !IsRetracting && !IsSwinging);
}

bool AGrappleBelt::CanUnhook()
{
	return (FiredHook && !IsRetracting || IsSwinging);
}

FVector AGrappleBelt::GetCableAttachPointBeltLocation()
{
	return CableAttachPointBelt->GetComponentLocation();
}

// Called every frame
void AGrappleBelt::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

bool AGrappleBelt::FireHook(FVector Location)
{
	// Fire hook projectile
	if (GrappleHookProjectileClass)
	{
		if (HookFiredSound)
		{
			UGameplayStatics::PlaySound2D(this, HookFiredSound, 1.0f);
		}

		FiredHook = true;
		HookCooldownTimer = GrappleBeltFireCooldown;
		FireNoHitTimer = GrappleBeltFireNoHitTimerDefault;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		//SpawnParams.bNoFail = true;
		FVector ProjectileSpawnLocation = CableAttachPointBelt->GetComponentLocation() + (CableAttachPointBelt->GetComponentLocation().ForwardVector * 70);
		GrappleHookProjectile = GetWorld()->SpawnActor<AGrappleHookProjectile_CPP>(GrappleHookProjectileClass, ProjectileSpawnLocation, FRotator::ZeroRotator, SpawnParams);

		if (GrappleHookProjectile)
		{
			FVector DifferenceVec = Location - GrappleHookProjectile->GetActorLocation();
			GrappleHookProjectile->SetActorRotation(DifferenceVec.ToOrientationRotator());

			/**
			 * This projectile will never need to generate overlap events, so disable them
			 * in case overlap events were not disabled on the projectile's static mesh
			 */
			GrappleHookProjectile->GetProjectileMesh()->SetGenerateOverlapEvents(false);

			// Fire hook projectile
			GrappleHookProjectile->SetOwner(this);
			GrappleHookProjectile->SetProjectileVelocity(GrappleBeltHookProjectileVelocity);
			GrappleHookProjectile->AddImpulse();

			// Attach grapple cable to the projectile
			GrappleCable->AttachToComponent(GrappleHookProjectile->CableAttachmentPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

			return true;
		}
		else
		{
			return false;
		}
	}

	else
	{
		return false;
	}
}

void AGrappleBelt::UnHook()
{
	// Set FiredHook to false in case UnHook() was called if OnHookLand() did not previously set it to false
	FiredHook = false;

	IsSwinging = false;
		
	// "Disable" FireNoHitTimer since the grapple hook projectile is no longer being fired
	FireNoHitTimer = 0.0f;

	RetractHook();
}

void AGrappleBelt::RetractHook()
{
	if (GrappleHookProjectile)
	{
		if (HookRetractSound)
		{
			UGameplayStatics::PlaySound2D(this, HookRetractSound, 1.0f);
		}

		IsRetracting = true;

		ProjectilePhysicsDisableTimer = ProjectilePhysicsDisableTimerDefault;
		/**
		 * GrappleHookProjectile movement was previously halted by setting simulate physics to false.
		 * Before applying forces to the projectile, physics simulation must first be turned on 
		 */
		GrappleHookProjectile->GetProjectileMesh()->SetSimulatePhysics(true);

		// Disable calling of OnHit on the projectile if it hits an actor while retracting
		GrappleHookProjectile->GetProjectileMesh()->SetNotifyRigidBodyCollision(false);

		FVector DifferenceVec = SwingPoint - CableAttachPointBelt->GetComponentLocation();

		// Retract hook back to grapple belt
		GrappleHookProjectile->SetActorRotation(DifferenceVec.ToOrientationRotator());
		GrappleHookProjectile->SetProjectileVelocity(-GrappleBeltHookProjectileVelocity / 3);
		GrappleHookProjectile->AddImpulse();
	}

	// Reset cable length
	GrappleCable->CableComponent->CableLength = DefaultCableLength;
}

void AGrappleBelt::FinishHookRetract()
{
	IsRetracting = false;

	// Detach cable before projectile is destroyed
	GrappleCable->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// Grapple hook projectile has retracted
	GrappleHookProjectile->Destroy();

	// Timer has successfully been retracted, so disable "timer" that disables projectile physics if hook is stuck on geometry
	ProjectilePhysicsDisableTimerDefault = 0;
}

void AGrappleBelt::OnHookLand(UPrimitiveComponent* HitComponent, AActor* HitActor, FVector NormalImpulse, const FHitResult& HitResult)
{
	FiredHook = false;

	// Check if hit actor is hookable
	if (HitActor->ActorHasTag(HookableActorTag) || HitComponent->ComponentHasTag(HookableActorTag))
	{	
		if (HookLandSuccessfulSound)
		{
			UGameplayStatics::PlaySound2D(this, HookLandSuccessfulSound, 1.0f);
		}

		// Make sure hook is not retracted prematurely by "disabling" this timer variable 
		FireNoHitTimer = 0;

		IsSwinging = true;

		// Stop movement of projectile
		//GrappleHookProjectile->GetProjectileMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
		GrappleHookProjectile->GetProjectileMesh()->SetSimulatePhysics(false);

		// Set swing point
		SwingPoint = GrappleHookProjectile->CableAttachmentPoint->GetComponentLocation();

		// Calculate cable length, and shorten it for small initial speed boost
		CableLength = FVector::Dist(SwingPoint, CableAttachPointBelt->GetComponentLocation());
		CableLength -= CableShorteningAfterHook;

		GrappleCable->CableComponent->CableLength = CableLength - CableVisualShortening;
	}
	else
	{
		if (HookLandFailedSound)
		{
			UGameplayStatics::PlaySound2D(this, HookLandFailedSound, 1.0f);
		}

		UnHook();
	}
}

void AGrappleBelt::HandleSwingOwner(ACharacter* GrappleOwner, float DeltaTime)
{	
	// Note: These functions will be ticked (from player character)
	HandleHookCooldown(DeltaTime);
	ApplyForces(GrappleOwner);
	Cosmetics();
}

void AGrappleBelt::Cosmetics()
{
	// Before grapple belt is fired, adjust cable location to avoid
	// glitching of cable physics when grapple belt is suddenly fired
	if (CanFire())
	{
		GrappleCable->SetActorLocation(CableAttachPointBelt->GetComponentLocation());
	}

	// If grapple hook projectile has glitched and despawned as a result, treat cable as being retracted
	if (FiredHook && !GrappleHookProjectile)
	{
		FinishHookRetract();
	}

	/*else if (FiredHook && GrappleHookProjectile)
	{
		GrappleCable->SetActorLocation(GrappleHookProjectile->CableAttachmentPoint->GetComponentLocation());
	}*/

	// Retract grapple hook projectile by continuously applying force towards GrappleBelt
	if (IsRetracting)
	{
		FVector GrappleHookRetractForce = CableAttachPointBelt->GetComponentLocation() - GrappleHookProjectile->GetActorLocation();
		GrappleHookProjectile->AddForce( (GrappleHookRetractForce * GrappleHookProjectileRetractForceMultiplier) );

		if (FVector::Distance(GrappleHookProjectile->GetActorLocation(), CableAttachPointBelt->GetComponentLocation()) < GrappleHookProjectileRetractFinishDistance)
		{
			FinishHookRetract();
		}
	}

	// If the hook has been fired or the player is swinging, the cable should be visible
	GrappleCable->CableComponent->SetVisibility( (IsSwinging || FiredHook || IsRetracting) );
}

void AGrappleBelt::ApplyForces(ACharacter* GrappleOwner)
{
	if(IsSwinging)
	{ 
		/* Note: The goal here is not an accurate physics simulation
		 * The goal here is to make a fluid feeling, fun grapple tool
		 */

		// Only apply force if the cable attach point on the player is lower than the 
		// point from which the player is swinging
		bool SwingingBelowPoint = CableAttachPointBelt->GetComponentLocation().Z < SwingPoint.Z;
		if (SwingingBelowPoint)
		{
			/**
			 * Create scaling force factor based on player's proximity to the swing point in the x, y
			 * plane. Essentially, the lower the player is on the sphere of rotation around the swing point,
			 * the more force will be applied to them. Value lies between 0 and 1
			 */
			float ForceScalingFactor = FVector::Dist2D(CableAttachPointBelt->GetComponentLocation(), SwingPoint) / CableLength;
			// Currently, ForceScalingFactor is greater the further up the sphere of rotation the player is. Invert this
			ForceScalingFactor = 1.0f - ForceScalingFactor;
			// Ensure ForceScalingFactor is between 0 and 1
			ForceScalingFactor = FMath::Clamp(ForceScalingFactor, 0.0f, 1.0f);

			/* First, add a swinging force to owner (applied in the direction of GrappleBelt to swinging point)  */
			// Vector from swing to belt
			FVector SwingToBeltVec = CableAttachPointBelt->GetComponentLocation() - SwingPoint;
			FVector SwingToBeltVecNormalized = SwingToBeltVec.GetSafeNormal();

			// Dot product between vector pointing from swing to belt and GrappleBelt/GrappleOwner's velocity (should be the same)
			float DotBetweenBeltVecAndVel = FVector::DotProduct(GrappleOwner->GetVelocity(), SwingToBeltVec);

			FVector CentripetalForce = SwingToBeltVecNormalized * DotBetweenBeltVecAndVel;
		
			// Invert vector so that it points from the belt to the swing point while scaling it by a constant
			CentripetalForce *= -CentripetalForceScalingFactor;
			GrappleOwner->GetCharacterMovement()->AddForce( (CentripetalForce * ForceScalingFactor) );
			

			/* Add boost according to player's input */
			FVector OwnerViewForwardVector = GrappleOwner->GetViewRotation().Vector();
			FVector OwnerInput = GrappleOwner->GetCharacterMovement()->GetLastInputVector();
			float BoostFloat;

			if (RestrictBoostToForwardDirection)
			{
				BoostFloat = FVector::DotProduct(OwnerInput.GetSafeNormal(), OwnerViewForwardVector);
				BoostFloat *= FVector::DotProduct(CableAttachPointBelt->GetComponentVelocity().GetSafeNormal(), OwnerInput.GetSafeNormal());
			}
			else
			{
				BoostFloat = FVector::DotProduct(CableAttachPointBelt->GetComponentVelocity().GetSafeNormal(), OwnerInput.GetSafeNormal());
			}

			BoostFloat *= BoostMultiplier;
			FVector BoostForce = (CableAttachPointBelt->GetComponentVelocity().GetSafeNormal() * BoostFloat).GetClampedToMaxSize(BoostMax);

			GrappleOwner->GetCharacterMovement()->AddForce( (BoostForce * ForceScalingFactor) );
		}
		
		/* Add counter impulse to prevent cable from stretching significantly */
		float DistBeltToSwing = FVector::Dist(CableAttachPointBelt->GetComponentLocation(), SwingPoint);
		DistBeltToSwing -= CableLength;
		// Do not allow DistBeltSwing to be negative (so that force is always applied in direction of cable)
		DistBeltToSwing = FMath::Max(DistBeltToSwing, 0.0f);
		// Find unit vector from belt to swing point
		FRotator DirBeltToSwing = UKismetMathLibrary::FindLookAtRotation(CableAttachPointBelt->GetComponentLocation(), SwingPoint);

		FVector CounterStretchForce = CounterStretchForceMultiplier * DistBeltToSwing * DirBeltToSwing.Vector();

		GrappleOwner->GetCharacterMovement()->AddForce(CounterStretchForce);
	}
}


