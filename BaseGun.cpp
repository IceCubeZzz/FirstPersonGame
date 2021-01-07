// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#include "BaseGun.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshSocket.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h" 

// Sets default values
ABaseGun::ABaseGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMeshStatic = CreateDefaultSubobject<UStaticMeshComponent>("Gun Static Mesh");
	WeaponMeshSkeletal = CreateDefaultSubobject<USkeletalMeshComponent>("Gun Skeletal Mesh");

	RootComponent = CreateDefaultSubobject<USceneComponent>("Root Component");

	SetRootComponent(RootComponent);

	if (WeaponMeshStatic->IsActive())
	{
		WeaponMeshStatic->SetCastShadow(false);
	}
	if (WeaponMeshSkeletal->IsActive())
	{
		WeaponMeshStatic->SetCastShadow(false);
	}
}

// Called when the game starts or when spawned
void ABaseGun::BeginPlay()
{
	Super::BeginPlay();

	// Setup component attachment order
	WeaponMeshSkeletal->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	WeaponMeshStatic->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	if (WeaponMeshStatic->IsActive())
	{
		// WeaponMeshSkeletal is checked for an assigned mesh in case WeaponMeshStatic does not have an assigned mesh
		if(WeaponMeshStatic->GetStaticMesh())
		{ 
			StaticFireSocket = WeaponMeshStatic->GetSocketByName(FIRE_SOCKET);
		}
	}

	if (WeaponMeshSkeletal->IsActive())
	{
		if (WeaponMeshSkeletal->SkeletalMesh)
		{
			SkeletalFireSocket = WeaponMeshSkeletal->GetSocketByName(FIRE_SOCKET);
		}
	}
	
	CurrentAmmoTotal = (StartingAmmo >= MaxBackupAmmo) ? MaxBackupAmmo : StartingAmmo;
	/**
	 * If the max size of a clip is -1, CurrentAmmoClip will be the same as the starting ammo
	 */
	CurrentAmmoClip = (MaxClipAmmo > -1) ? MaxClipAmmo : StartingAmmo;

	if (MuzzleFlash)
	{
		if (SkeletalFireSocket)
		{
			MuzzleFlashParticleSystem = UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMeshSkeletal, FIRE_SOCKET, FVector::ZeroVector,
				FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false, EPSCPoolMethod::None, false);
		}
		if (StaticFireSocket)
		{
			MuzzleFlashParticleSystem = UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMeshStatic, FIRE_SOCKET, FVector::ZeroVector,
				FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false, EPSCPoolMethod::None, false);
		}

		/*if (ensure(MuzzleFlashParticleSystem))
		{
			MuzzleFlashParticleSystem->DeactivateSystem();
		}*/
	}
}

void ABaseGun::BeginReload()
{
	int AmmoToRefill = (CurrentAmmoTotal >= MaxClipAmmo) ? MaxClipAmmo : CurrentAmmoTotal;
	CurrentAmmoClip = AmmoToRefill;
	CurrentAmmoTotal -= AmmoToRefill;
}

void ABaseGun::PlayFireEffects()
{
	PlayFireSound();
	PlayGunFireAnimation();
	PlayMuzzleFlash();
}

void ABaseGun::PlayMuzzleFlash()
{
	if (MuzzleFlashParticleSystem)
	{
		MuzzleFlashParticleSystem->ActivateSystem();
	}
}

void ABaseGun::PlayGunFireAnimation()
{
	if(WeaponMeshSkeletal->IsActive())
	{ 
		UAnimInstance* AnimationInstance = WeaponMeshSkeletal->GetAnimInstance();
		if (AnimationInstance && WeaponFireAnimation)
		{
			AnimationInstance->PlaySlotAnimationAsDynamicMontage(WeaponFireAnimation, "GunAction", 0.0f, 0.0f);
		}
	}
}

void ABaseGun::PlayFireSound()
{
	if(FireSound)
	{ 
		// Play 2D sound 
		UGameplayStatics::PlaySound2D(this, FireSound, FireSoundMultiplier);

		// Play 3D sound at gun location (while more realistic, it sounds less pleasing to have most of the fire sound playing in only one speaker)
		// Note: Location of root component is close enough to muzzle for spawning sound attached
		/*UGameplayStatics::SpawnSoundAttached(FireSound, GetRootComponent(), "", FVector::ZeroVector,
			EAttachLocation::SnapToTarget, true, FireSoundMultiplier);*/
	}
}

bool ABaseGun::Fire()
{
	if (FireCooldownTimer < 0 && CurrentAmmoClip > 0 && IntermediateReloadCompleted)
	{
		FireTowardsLocation();
		--CurrentAmmoClip;
		FireCooldownTimer = 60.0f / ProjectilesPerSecond;

		if (MuzzleFlashParticleSystem)
		{
			MuzzleFlashTimer = MuzzleFlashTimeBeforeDisable;
		
		}
		if (TimeToReloadIntermediate > 0.0f)
		{
			// Gun must now be prepped for next shot
			IntermediateReloadCompleted = false;
		}

		return true;
	}
	// If the gun cannot be fired at this time, false is returned to signify that no projectile was fired
	else
	{
		return false;
	}
}

bool ABaseGun::BeginReloadAnimation()
{
	// Play weapon reload animations here (currently none exist)
	
	// If the player currently has a full clip, do not let them reload
	if (CurrentAmmoClip == MaxClipAmmo)
	{
		return false;
	}
	else 
		return true;
}

void ABaseGun::SetLocationToFireTowards(FVector NewLocation)
{
	LocationToFireTowards = NewLocation;
}

void ABaseGun::FireTowardsLocation()
{
	// If we have not set a location to fire towards, fire forward instead
	// It is almost guaranteed that if a manual location to fire towards is set, this location
	// will not exactly be the zero vector
	if (LocationToFireTowards.IsZero())
	{
		//FireSocketForwardVector = (GunMesh->GetSocketRotation(FIRE_SOCKET)).Vector();
		FireSocketForwardVector = GetFireSocketTransform().Rotator().Vector();
		LocationToFireTowards = FireSocketForwardVector * 10000;
	}
	if (ProjectileToFire == nullptr)
	{
		CreateProjectileToFire();
	}
	if (ProjectileToFire)
	{
		FHitResult LineTraceHit;
		FCollisionQueryParams LineTraceQueryParams;
		LineTraceQueryParams.bReturnPhysicalMaterial = true;
		LineTraceQueryParams.AddIgnoredActor(GetOwner());
		LineTraceQueryParams.AddIgnoredActor(this);
		
		/**
		 * If line tracing should be used rather than physical collision for projectile damage, then
		 * conduct line trace
		 */ 
		if (ShouldLineTraceTest)
		{
			GetWorld()->LineTraceSingleByChannel(LineTraceHit, LineTraceStartLocation, LocationToFireTowards,
				ECollisionChannel::ECC_Visibility, LineTraceQueryParams);

			/**
			 * Calculate the time until projectile should self destruct by using projectile's velocity value
			 * and the distance from projectile and LineTraceHit if an actor was hit, or LocationToFireTowards
			 * if no actor was hit
			 */
			float ProjectileTravelDistance = 0.0f;

			if (LineTraceHit.IsValidBlockingHit())
			{
				LocationToFireTowards = LineTraceHit.Location;
				ProjectileTravelDistance = (LineTraceHit.Location - ProjectileToFire->GetActorLocation()).Size();
			}
			else
			{
				ProjectileTravelDistance = (LocationToFireTowards - ProjectileToFire->GetActorLocation()).Size();

				/*GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("hitfailed"));
				if (LineTraceHit.GetActor())
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, LineTraceHit.GetActor()->GetName());
				}*/
			}

			float ProjectileTravelTime = ProjectileTravelDistance / ProjectileVelocity;

			ProjectileToFire->ProjectileAutoDestroy(ProjectileTravelTime, LineTraceHit);
		}
		else
		{
			// If line tracing is not being used, enable collision for projectile
			ProjectileToFire->GetProjectileMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ProjectileToFire->GetProjectileMesh()->SetNotifyRigidBodyCollision(true);
		}

		ShotDirection = LocationToFireTowards - ProjectileToFire->GetActorLocation();

		// Calculate projectile spread
		float HalfRad = FMath::DegreesToRadians(ProjectileSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

		ProjectileToFire->SetDamageFloat(WeaponDamage);
		ProjectileToFire->SetActorRotation(ShotDirection.ToOrientationQuat());
		ProjectileToFire->SetProjectileVelocity(ProjectileVelocity);
		ProjectileToFire->AddImpulse();
		
		ProjectileToFire = nullptr;
		PlayFireEffects();
	}
}

void ABaseGun::FirePoolProjectile(ABaseProjectile* PoolProjectileToFire)
{
	ProjectileToFire = PoolProjectileToFire;

	FTransform FireTransform = GetFireSocketTransform();

	ProjectileToFire->SetActorTransform(FireTransform);

	FireTowardsLocation();
} 

void ABaseGun::CreateProjectileToFire()
{
	FActorSpawnParameters FireParams;
	FireParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	/** 
	 * Instigator reference must be passed down to the projectile since it will be the object applying
	 * damage. The instigator's controller reference will be passed by the projectile to the AActor it damages
	 */
	FireParams.Instigator = this->GetInstigator();

	FTransform FireTransform = GetFireSocketTransform();
	/*if (ShouldFireFromSocket)
	{
		FireTransform = GetFireSocketTransform();
	}
	else
	{
		FireTransform = FTransform(FRotator::ZeroRotator, ProjectileNonSocketSpawnPoint);
	}*/

	ProjectileToFire = GetWorld()->SpawnActor<ABaseProjectile>(ProjectileType, FireTransform, FireParams);

	if (ProjectileToFire)
	{
		// Ensure projectile spawns in front of the gun / character
		FVector MinBound, MaxBound;
		ProjectileToFire->GetProjectileMesh()->GetLocalBounds(MinBound, MaxBound);
		FireTransform.SetLocation(FireTransform.GetLocation() + MaxBound / 2);
	}
}

FTransform ABaseGun::GetFireSocketTransform()
{
	// Get transform of the socket we will fire the projectile from 
	FTransform FireTransform;
	if(StaticFireSocket)
	{ 
		if (StaticFireSocket->GetSocketTransform(FireTransform, WeaponMeshStatic))
		{
			return FireTransform;
		}
	}
	if(SkeletalFireSocket)
	{ 
		return SkeletalFireSocket->GetSocketTransform(WeaponMeshSkeletal);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Gun firing socket not set"));
		return GetActorTransform();
	}
}

// Called every frame
void ABaseGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FireCooldownTimer -= DeltaTime;

	if (MuzzleFlashTimer > 0)
	{
		MuzzleFlashTimer -= DeltaTime;

		if (MuzzleFlashTimer < 0 || FMath::IsNearlyZero(MuzzleFlashTimer))
		{
			MuzzleFlashTimer = 0;
			MuzzleFlashParticleSystem->DeactivateSystem();
		}
	}
}

void ABaseGun::SetProjectileType(TSubclassOf<ABaseProjectile> NewProjectileType)
{
	ProjectileType = NewProjectileType;
}

float ABaseGun::GetProjectileVelocity()
{
	return ProjectileVelocity;
}

int ABaseGun::GetCurrentAmmo()
{
	return CurrentAmmoClip;
}

int ABaseGun::GetTotalAmmo()
{
	return CurrentAmmoTotal;
}

void ABaseGun::SetLineTraceTestStart(FVector LineTraceLocation, FRotator LineTraceRotation)
{
	LineTraceStartLocation = LineTraceLocation;
	LineTraceStartRotation = LineTraceRotation;
}

bool ABaseGun::GetShouldLineTraceTest()
{
	return ShouldLineTraceTest;
}

