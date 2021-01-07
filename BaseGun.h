// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.h"
#include "Components/SceneComponent.h"
#include "Weapon_CPP.h"
#include "BaseGun.generated.h"

#define FIRE_SOCKET "Fire"

UCLASS()
class DREAMREALM_API ABaseGun : public AWeapon_CPP
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseGun();

protected:
	UPROPERTY(EditDefaultsOnly)
	// Type of the projectile to be created before being fired by this gun
	TSubclassOf<ABaseProjectile> ProjectileType;

	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	int ProjectilesPerFire = 1;
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	float ProjectilesPerSecond = 120;
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	float ProjectileSpread = .5f;
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	float ProjectileVelocity = 1500.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	/**
	 * Maximum ammo in a clip
	 * -1 signifies that the gun never has to be reloaded.
	 */
	int MaxClipAmmo = -1;
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	/**
	 * Maximum backup ammo that can be held for this gun
	 * -1 signifies that the gun has no maximum ammo count.
	 */
	int MaxBackupAmmo = -1;
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	int StartingAmmo = MAX_int32;
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	UParticleSystem* MuzzleFlash;
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	// Time for which muzzle flash particle effect remains visible
	float MuzzleFlashTimeBeforeDisable = .1f;

	// Ammo in current magazine
	int CurrentAmmoClip;
	// Current total ammo
	int CurrentAmmoTotal;

	// Location towards which the projectile will be fired
	FVector LocationToFireTowards;
	// Direction the projectile will fly after taking into account accuracy calculations
	FVector ShotDirection;

	ABaseProjectile* ProjectileToFire;

	USceneComponent* RootComponent;

	UParticleSystemComponent* MuzzleFlashParticleSystem;

	/**
	 * These could be removed in place of a single UObject pointer, but this would require 
	 * casting the UObject to the correct socket type. Therefore, including references for 
	 * both types of sockets is a slight optimization
	 */
	const UStaticMeshSocket* StaticFireSocket;
	const USkeletalMeshSocket* SkeletalFireSocket;
	FVector FireSocketForwardVector;
	
	FTimerHandle ContinuousFireHandle;

	float MuzzleFlashTimer = 0.0f;

	float FireCooldownTimer = 0.0f;

	/**
	 * Should line trace be used during fire? Because projectiles are by default spawned at the gun's fire socket,
	 * when the player is shooting at enemies close to them, projectiles may miss despite the cross hair being on the enemy
	 * due to the projectile spawning off center. Line tracing allows correction of this problem 
	 */
	UPROPERTY(EditDefaultsOnly)
	bool ShouldLineTraceTest = true;
	FVector LineTraceStartLocation;
	FRotator LineTraceStartRotation;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void PlayMuzzleFlash();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual bool Fire() override;

	UFUNCTION(BlueprintCallable)
	virtual bool BeginReloadAnimation() override;

	/**
	 * Reloads current clip
	 */
	virtual void BeginReload() override;

	// Set the location towards which the projectile will be fired
	void SetLocationToFireTowards(FVector NewLocation) override;

	// Fire projectile in given direction
	virtual void FireTowardsLocation() override;

	UFUNCTION(BlueprintCallable)
	// Fire projectile from a pool of projectiles
	virtual void FirePoolProjectile(ABaseProjectile* PoolProjectileToFire);

	// Create new projectile to fire
	virtual void CreateProjectileToFire();

	// Get FireSocket transform so that projectile can be moved to right spot before firing
	virtual FTransform GetFireSocketTransform();

	// Plays effects associated with gun firing. Includes both sound and particle effects
	virtual void PlayFireEffects();

	void PlayGunFireAnimation() override;

	void PlayFireSound();

	void SetProjectileType(TSubclassOf<ABaseProjectile> NewProjectileType);

	// Returns the velocity of the projectiles fired by this gun
	float GetProjectileVelocity();

	// Returns CurrentAmmoClip 
	int GetCurrentAmmo() override;

	// Returns CurrentAmmoTotal
	int GetTotalAmmo() override;

	void SetLineTraceTestStart(FVector LineTraceLocation, FRotator LineTraceRotation);

	bool GetShouldLineTraceTest();
};
