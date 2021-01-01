// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#include "BaseProjectile.h"
#include "Engine/SkeletalMesh.h"
#include "InterfaceRecieveDamage.h"
#include "PlayerCharacter_CPP.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h" 
#include "Kismet/KismetMathLibrary.h" 
#include "DrawDebugHelpers.h" 

// Sets default values
ABaseProjectile::ABaseProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("ProjectileMesh");
	ProjectileMesh->SetGenerateOverlapEvents(false);

	// Add default key value pairs so that values can easily be assigned in the editor
	for (int i = 0; i < 10; i++)
	{
		ParticleEffectsOnHit.Add(static_cast<EPhysicalSurface>(i), NULL);
	}
}

// Called when the game starts or when spawned
void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	/**
	 * Since some guns use line tracing for damage instead of physical projectile collision,
	 * disable hit query by default, and enable it later if necessary
	 */
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	ProjectileMesh->SetNotifyRigidBodyCollision(false);
}

void ABaseProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// If the overlapped actor is capable of receiving damage, damage will be applied to that actor
	if (OtherActor->GetClass()->ImplementsInterface(UInterfaceRecieveDamage::StaticClass()))
	{
		FVector HitDirection = SweepResult.ImpactPoint - GetActorLocation();
		HitDirection = HitDirection.GetSafeNormal();

		// If there is no explicit damage type, the actor will simply have health subtracted
		UGameplayStatics::ApplyPointDamage(OtherActor, ProjectileDamage, HitDirection, SweepResult, this->GetInstigatorController(), this, nullptr);
	}
	Destroy();
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	HitActor = OtherActor;
	ProjectileHit = Hit;

	ApplyDamage();
	
	PlayParticleEffectsOnHit();
	Destroy();
}

void ABaseProjectile::ProjectileAutoDestroy(float TimeUntilDestroy, const FHitResult& Hit)
{
	CurrentTimeUntilDestroy = TimeUntilDestroy;
	
	HitActor = Hit.GetActor();
	ProjectileHit = Hit;
}

void ABaseProjectile::PlayParticleEffectsOnHit()
{
	switch (ProjectileHit.PhysMaterial->SurfaceType)
	{
		// Default_Ground
		case EPhysicalSurface::SurfaceType_Default:
			if (ParticleEffectsOnHit.Contains(EPhysicalSurface::SurfaceType_Default) && ParticleEffectsOnHit[EPhysicalSurface::SurfaceType_Default])
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleEffectsOnHit[EPhysicalSurface::SurfaceType_Default], FTransform(UKismetMathLibrary::MakeRotFromZ(ProjectileHit.ImpactNormal), ProjectileHit.ImpactPoint));
			}
			break;
	}
}

// Called every frame
void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentTimeUntilDestroy > 0)
	{
		CurrentTimeUntilDestroy -= DeltaTime;
		if (CurrentTimeUntilDestroy <= 0)
		{
			ProjectileDestroy();
		}
	}
}

void ABaseProjectile::SetProjectileMesh(UStaticMeshComponent* NewMesh)
{
	ProjectileMesh = NewMesh;
}

UStaticMeshComponent* ABaseProjectile::GetProjectileMesh()
{
	return ProjectileMesh;
}

void ABaseProjectile::SetDamageFloat(float Damage)
{
	ProjectileDamage = Damage;
}

void ABaseProjectile::AddImpulse()
{
	FVector ImpulseVector = this->GetActorForwardVector() * ProjectileVelocity;
	ProjectileMesh->AddImpulse(ImpulseVector * ProjectileMesh->GetMass());
}

void ABaseProjectile::SetProjectileVelocity(float Velocity)
{
	ProjectileVelocity = Velocity;
}

float ABaseProjectile::GetProjectileVelocity()
{
	return ProjectileVelocity;
}

void ABaseProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ProjectileMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnOverlap);
	ProjectileMesh->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
}

void ABaseProjectile::ApplyDamage()
{	
	// If the hit actor is capable of receiving damage, damage will be applied to that actor
	if (HitActor->GetClass()->ImplementsInterface(UInterfaceRecieveDamage::StaticClass()))
	{
		FVector HitDirection = ProjectileHit.ImpactPoint - GetActorLocation();
		HitDirection = HitDirection.GetSafeNormal();

		// If there is no explicit damage type, the actor will simply have health subtracted
		UGameplayStatics::ApplyPointDamage(HitActor, ProjectileDamage, HitDirection, ProjectileHit, this->GetInstigatorController(), this, nullptr);

		/**
		 * If this projectile has an instigator, attempt to cast it to APlayerCharacter_CPP
		 * so that a hitmarker can be displayed on the player's UI
		 */
		if (this->GetInstigator())
		{
			APlayerCharacter_CPP* InstigatorRef = Cast<APlayerCharacter_CPP>(this->GetInstigator());
			if (InstigatorRef)
			{
				InstigatorRef->DisplayHitmarker();
			}
		}
	}
}

void ABaseProjectile::ProjectileDestroy()
{
	/**
	 *  If the line trace hit an actor, attempt to apply damage and play particle effects
	 */
	if (HitActor && ProjectileHit.IsValidBlockingHit())
	{
		ApplyDamage();
		PlayParticleEffectsOnHit();
	}

	Destroy();
}

void ABaseProjectile::EnableCollisionEvents()
{
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProjectileMesh->SetNotifyRigidBodyCollision(true);
}

