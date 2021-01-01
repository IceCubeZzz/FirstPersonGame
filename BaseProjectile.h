// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

UCLASS()
class DREAMREALM_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	UStaticMeshComponent* ProjectileMesh;

	float ProjectileVelocity = 0;

	AController* Instigator;

	float CurrentTimeUntilDestroy;

	AActor* HitActor;
	FHitResult ProjectileHit;

	void ApplyDamage();
	void ProjectileDestroy();
	void EnableCollisionEvents();
	
	/**
	 * TMap containing UParticleSystems so that particle effects may be played when this projectile hits an actor.
	 */ 
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	TMap<TEnumAsByte<EPhysicalSurface>, UParticleSystem*> ParticleEffectsOnHit;

	/* DO NOT SET. Variable is set by the gun from which this projectile is fired
	*  so that buffs or debuffs can be applied to each projectile from the gun
	*/
	float ProjectileDamage = 0;

	// Currently unused. Consider implementing in future
	//UDamageType* ProjectileDamageType;
		
	UFUNCTION()
	/** 
	 * Apply damage to the actor that was overlapped by this projectile (if it is capable of receiving damage)
	 * The overlap event is used to damage to the player, so that the player's movement is 
	 * unobstructed by the projectile's momentum
	 */
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void PlayParticleEffectsOnHit();

	UFUNCTION()
	/**
	 * Apply damage to the actor that was hit by this projectile (if it is capable of receiving damage)
	 * The hit event is used to damage enemies
	 */	
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	
	/**
	 * If the gun from which this particle is fired uses line tracing (rather than physical projectile collisions),
	 * use this method to auto destroy projectile after a certain amount of time. (spawning particle system after destroy is optional)
	 */
	void ProjectileAutoDestroy(float TimeUntilDestroy, const FHitResult& Hit);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetProjectileMesh(UStaticMeshComponent* NewMesh);

	UStaticMeshComponent* GetProjectileMesh();
	
	// Set projectile damage
	virtual void SetDamageFloat(float Damage);

	// Adds impulse to the projectile, "firing" it 
	virtual void AddImpulse();

	// Set the velocity with which this projectile will be "fired" when AddImpulse() is called 
	void SetProjectileVelocity(float Velocity);

	float GetProjectileVelocity();

	virtual void PostInitializeComponents() override;
};
