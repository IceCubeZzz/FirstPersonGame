// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrappleBelt.generated.h"

#define IGNORE_ALL ECollisionChannel::ECC_GameTraceChannel5
UCLASS()
class DREAMREALM_API AGrappleBelt : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrappleBelt();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	// Velocity given to the belt hook projectile as it is fired
	float GrappleBeltHookProjectileVelocity = 2000.0f;
	UPROPERTY(EditDefaultsOnly)
	USoundCue* HookFiredSound;
	UPROPERTY(EditDefaultsOnly)
	USoundCue* HookRetractSound;
	UPROPERTY(EditDefaultsOnly)
	// Sound played when hook has hit a hookable actor
	USoundCue* HookLandSuccessfulSound;
	UPROPERTY(EditDefaultsOnly)
	// Sound played when hook hits a non hookable actor
	USoundCue* HookLandFailedSound;
	UPROPERTY(EditDefaultsOnly)
	// Tag on actors that are hookable
	FName HookableActorTag = "Hookable";
	UPROPERTY(EditDefaultsOnly)
	// Cooldown after firing grapple belt (regardless of whether or not hook lands on valid surface)
	float GrappleBeltFireCooldown = 1.0f;
	UPROPERTY(EditDefaultsOnly)
	// Maximum time after firing grapple belt before hook is retracted
	float GrappleBeltFireNoHitTimerDefault = 1.0f;
	UPROPERTY(EditDefaultsOnly)
	// If hook projectile cannot be retracted within this time (it may be stuck on an object), disable its collision so it can be retracted
	float ProjectilePhysicsDisableTimerDefault = 2.0f;
	UPROPERTY(EditDefaultsOnly)
	// Length the cable shortens after successful hook. Gives slight speed boost and looks aesthetically pleasing
	float CableShorteningAfterHook = 100.0f;
	UPROPERTY(EditDefaultsOnly)
	// Distance shortening applied to GrappleCable's cable component. Serves only as a visual effect to simulate cable tension
	float CableVisualShortening = 200.0f;
	UPROPERTY(EditDefaultsOnly)
	// Force boost multiplier applied to direction player input is facing. Makes grappling more fluid and fast
	float BoostMultiplier = 200.0f;
	UPROPERTY(EditDefaultsOnly)
	// Maximum value of the boost that is applied to player while grappling
	float BoostMax = 6000.0f;
	UPROPERTY(EditDefaultsOnly)
	// Only allow the player to boost in forward direction
	bool RestrictBoostToForwardDirection = false;
	UPROPERTY(EditDefaultsOnly)
	// Scales counter stretch force. Lower values mean that stretching will be fixed more slowly
	float CounterStretchForceMultiplier = 6000.0f;
	UPROPERTY(EditDefaultsOnly)
	// Maximum distance between grapple hook projectile and grapple belt at which grapple cable is hidden
	float GrappleHookProjectileRetractFinishDistance = 150.0f;
	UPROPERTY(EditDefaultsOnly)
	// Scales the force with which grapple hook projectile is retracted
	float GrappleHookProjectileRetractForceMultiplier = 25.0f;
	UPROPERTY(EditDefaultsOnly)
	// Resets cable length to this length so that the visual length of the cable when firing hook looks constant
	float DefaultCableLength = 600.0f;
	UPROPERTY(EditDefaultsOnly)
	// Scales the centripetal force applied to the player
	float CentripetalForceScalingFactor = 1.5f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AGrappleCable_CPP> GrappleCableClass;
	UPROPERTY()
	AGrappleCable_CPP* GrappleCable;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AGrappleHookProjectile_CPP> GrappleHookProjectileClass;
	UPROPERTY()
	AGrappleHookProjectile_CPP* GrappleHookProjectile;

	UPROPERTY(Category = "Setup", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneComponent;
	UPROPERTY(Category = "Setup", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* CableAttachPointBelt;

	FVector SwingPoint;
	float CableLength = 0.0f;
	float HookCooldownTimer;
	float FireNoHitTimer;
	float ProjectilePhysicsDisableTimer;
	// Is the player currently swinging?
	bool IsSwinging;
	// Is the hook currently retracting?
	bool IsRetracting;
	// Has hook been fired and player is not yet swinging?
	bool FiredHook;

	void HandleHookCooldown(float DeltaTime);

	void ApplyForces(ACharacter* GrappleOwner);

	// Retract hook if the actor hit by the hook is not tagged as hookable
	void RetractHook();

	void FinishHookRetract();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void UnHook();

	// Fire hook towards given location. Returns true if hook successfully fired
	bool FireHook(FVector Location);

	// Called when hook projectile hits an actor
	void OnHookLand(UPrimitiveComponent* HitComponent, AActor* HitActor, FVector NormalImpulse, const FHitResult& HitResult);

	// Handles ticking of methods that allow player to swing 
	void HandleSwingOwner(ACharacter* GrappleOwner, float DeltaTime);

	void Cosmetics();

	bool CanFire();

	bool CanUnhook();

	FVector GetCableAttachPointBeltLocation();
};
