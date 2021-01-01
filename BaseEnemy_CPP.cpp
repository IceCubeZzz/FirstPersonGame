// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "BaseEnemy_CPP.h"
#include "PickupableItem_CPP.h"
#include "RagdollComponent.h"
#include "Components/SkeletalMeshComponent.h" 

// Sets default values
ABaseEnemy_CPP::ABaseEnemy_CPP()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RagdollComponent = CreateDefaultSubobject<URagdollComponent>(TEXT("Ragdoll Component"));
}

// Called when the game starts or when spawned
void ABaseEnemy_CPP::BeginPlay()
{
	Super::BeginPlay();

	RagdollComponent->RegisterComponent();

	DynamicMaterial = GetMesh()->CreateAndSetMaterialInstanceDynamicFromMaterial(0, GetMesh()->GetMaterial(0));
	SkeletalMeshReference = GetMesh();

	OnTakePointDamage.AddDynamic(this, &ABaseEnemy_CPP::OnPointDamage);
	OnTakeRadialDamage.AddDynamic(this, &ABaseEnemy_CPP::OnRadialDamage);
}

void ABaseEnemy_CPP::SetupWeapon(AWeapon_CPP* NewWeapon)
{
	Weapon = NewWeapon;
}

// Called every frame
void ABaseEnemy_CPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsDead && ShouldPhysReturn && !RagdollComponent->GetIsRagdoll())
	{
		PhysBlendReturn(DeltaTime);
	}
	if (RagdollDurationTimer > 0)
	{
		RagdollDurationTimer -= DeltaTime;
		if (RagdollDurationTimer <= 0)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Ragdoll Disabled");
			DisableRagdoll();
		}
	}
}

// Called to bind functionality to input
void ABaseEnemy_CPP::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

/*void ABaseEnemy_CPP::ApplyImpulseToBrokenPart(AActor* BodyPart, FName BoneName)
{
	USkeletalMeshComponent* BodyPartMesh = BodyPart->FindComponentByClass<USkeletalMeshComponent>();
	BodyPartMesh->AddImpulse(LastHitDamage * LastHitDirection.GetSafeNormal() * OnBodyPartDetachDamageImpulseConversion, BoneName);
}*/

void ABaseEnemy_CPP::TookDamage()
{
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue("TimeLastDamaged", GetWorld()->GetWorld()->TimeSeconds);
	}
}

void ABaseEnemy_CPP::OnPointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	TookDamage();
	/**
	 * Note: CurrentHealth is stored in PreviousHealth because ApplyDamage() deals damage based on the bone hit, but since this 
	 * value is not returned, we have to calculate it using PreviousHealth - CurrentHealth
	 */
	PreviousHealth = CurrentHealth;
	CurrentHealth = ApplyDamage(Damage, CurrentHealth, CurrentDamageMultiplier, HitLocation, BoneName, ShotFromDirection, GetMesh(), DamageCauser);
	if (CurrentHealth <= 0)
		StoreLastHitInfo(ShotFromDirection, HitLocation, (PreviousHealth - CurrentHealth), BoneName);
	ApplyTempImpulseToBone(Damage, FHitComponent, BoneName, HitLocation, ShotFromDirection);
	CheckIfDead();
}

void ABaseEnemy_CPP::OnRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, FHitResult HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	TookDamage();
	CurrentHealth = ApplyDamage(Damage, CurrentHealth, CurrentDamageMultiplier, HitInfo.Location, HitInfo.BoneName, HitInfo.Location - Origin, HitInfo.GetComponent(), DamageCauser);
	if (CurrentHealth <= 0)
		StoreLastHitInfo(HitInfo.Location - Origin, HitInfo.Location, (PreviousHealth - CurrentHealth), HitInfo.BoneName);
	ApplyTempRadialImpulseToBone(Damage, HitInfo.GetComponent(), HitInfo.BoneName, Origin, HitInfo.ImpactPoint - Origin/*FRotationMatrix::MakeFromX(HitInfo.ImpactPoint - Origin).Rotator().Vector()*/);
	CheckIfDead();
}

void ABaseEnemy_CPP::StoreLastHitInfo(FVector HitDirection, FVector HitLocation, float HitDamage, FName BoneName)
{
	LastHitDirection = HitDirection;
	LastHitDamage = HitDamage;
	LastHitLocation = HitLocation;
	LastHitBoneName = BoneName;
}

void ABaseEnemy_CPP::OnDeath()
{
	IsDead = true;

	// "Reset" RagdollDurationTimer in case the "timer" was recently set before death
	RagdollDurationTimer = 0.0f;
	DynamicMaterial->SetScalarParameterValue("IsDead", 1.0f);
	
	// Call OnDeath() on this enemy's ai controller
	ABaseAIController_CPP* AIController = Cast<ABaseAIController_CPP>(GetController());
	if (AIController)
	{
		AIController->OnDeath();
	}

	/*
	// Spawn weapon drop on death
	FVector SpawnLocation = Weapon->GetActorLocation();
	FRotator SpawnRotation = Weapon->GetActorRotation();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<APickupableItem_CPP>(SpawnLocation, SpawnRotation, SpawnParams);
	*/
	
	if (Weapon)
	{
		FDetachmentTransformRules DetatchRules = DetatchRules.KeepWorldTransform;
		Weapon->DetachFromActor(DetatchRules);
		Weapon->GetWeaponStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Weapon->GetWeaponStaticMeshComponent()->SetSimulatePhysics(true);
		Weapon->GetWeaponStaticMeshComponent()->SetEnableGravity(true);
	}

	ApplyImpulseToLastBone(LastHitDamage * OnDeathDamageImpulseConversion * LastHitDirection.GetSafeNormal(), LastHitLocation);
}

URagdollComponent* ABaseEnemy_CPP::GetRagdollComponent()
{
	return RagdollComponent;
}

void ABaseEnemy_CPP::EnableTempRagdoll(float RagdollDuration)
{
	EnableRagdoll();
	RagdollDurationTimer = RagdollDuration;
}

void ABaseEnemy_CPP::EnableRagdoll()
{
	RagdollComponent->EnableRagdollHumanoid();
	EnableRagdoll_BP();
}

void ABaseEnemy_CPP::OnBoneBreak(FName BoneName, USkeletalMeshComponent* SkeletalMesh)
{
	// Set correct material on broken part
	SkeletalMesh->SetMaterial(0, DynamicMaterial);
	/**
	 * This slot is reserved for a material depicting the inside of the enemy (the material is assigned to the vertices
	 * that were detached from the rest of the mesh)
	 */ 
	SkeletalMesh->SetMaterial(1, DynamicMaterial);
}

void ABaseEnemy_CPP::PhysBlendReturn(float DeltaTime)
{
	if (!bPaused)
	{
		if (SkeletalMeshReference->GetBodyInstance(SimulatePhysicsFromBone)->PhysicsBlendWeight > 0.f)
		{
			if (SkeletalMeshReference->GetBodyInstance(SimulatePhysicsFromBone)->PhysicsBlendWeight > SpeedExitingPhysicsSimulations * DeltaTime)
			{
				SkeletalMeshReference->AccumulateAllBodiesBelowPhysicsBlendWeight(SimulatePhysicsFromBone, SpeedExitingPhysicsSimulations * DeltaTime * -1.f, false);
			}
			else
			{
				SkeletalMeshReference->AccumulateAllBodiesBelowPhysicsBlendWeight(SimulatePhysicsFromBone, SkeletalMeshReference->GetBodyInstance(SimulatePhysicsFromBone)->PhysicsBlendWeight * -1.f, false);
			}
		}
		else
		{
			ShouldPhysReturn = false;

			if (SkeletalMeshReference->IsAnySimulatingPhysics())
				SkeletalMeshReference->SetAllBodiesBelowSimulatePhysics(SimulatePhysicsFromBone, false, true);
		}
	}
}

void ABaseEnemy_CPP::CheckIfDead()
{
	if (CurrentHealth <= 0 && !IsDead)
	{
		EnableRagdoll();
		OnDeath();
	}
}

void ABaseEnemy_CPP::ApplyImpulseToBone(FVector Impulse, FVector Location, FName BoneName)
{
	GetMesh()->AddImpulseAtLocation(Impulse, Location, BoneName);
}

void ABaseEnemy_CPP::ApplyImpulseToLastBone(FVector Impulse, FVector Location)
{
	GetMesh()->AddImpulseAtLocation(Impulse, Location, LastHitBoneName);
}

bool ABaseEnemy_CPP::ApplyTempImpulseToBone(float Damage, UPrimitiveComponent* PrimitiveComponent, FName Bone, FVector ImpulseLocation, FVector ImpulseDirection)
{
	ShouldPhysReturn = true;

	USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(PrimitiveComponent);
	ACharacter* Character = Cast<ACharacter>(PrimitiveComponent->GetOwner());

	float ImpulseStrength = 0.f;
	if (SkeletalMesh)
	{
		if (SkeletalMeshReference == SkeletalMesh)
		{
			float ImpulseScale = 1.0f;

			float* boneImpulseScale = ImpulseMultiplierPerBones.Find(Bone);
			if (boneImpulseScale != nullptr)
				ImpulseScale = *boneImpulseScale;

			ImpulseStrength = FMath::Clamp(
				((Damage * ImpulseScale) / SkeletalMesh->GetMass()) * (IsDead ? OnDeathDamageImpulseConversion : (1.f - SkeletalMeshReference->GetBodyInstance(SimulatePhysicsFromBone)->PhysicsBlendWeight)),
				0.f, MaxPhysImpulse);

			// Apply impulse to bone
			if (!bPaused)
			{

				if (!SkeletalMeshReference->IsAnySimulatingPhysics())
					SkeletalMeshReference->SetAllBodiesBelowSimulatePhysics(SimulatePhysicsFromBone, true, true);

				SkeletalMeshReference->AccumulateAllBodiesBelowPhysicsBlendWeight(SimulatePhysicsFromBone, 1.f, false);
			}
			else
			{
				if (!bApplyImpulseWhenPaused)
					return false;
			}
			if (SkeletalMeshReference->GetBodyInstance(Bone))
			{
				if (SkeletalMeshReference->GetBodyInstance(Bone)->bSimulatePhysics)
				{
					SkeletalMeshReference->AddImpulse(ImpulseDirection * ImpulseStrength, Bone, true);
				}
				else
				{
					SkeletalMeshReference->AddImpulse(ImpulseDirection * ImpulseStrength, SimulatePhysicsFromBone, true);
				}
			}

			return true;

		}
	}
	else
	{
		if (SkeletalMeshReference == PrimitiveComponent)
		{
			ImpulseStrength = FMath::Clamp(
				(Damage * OnDeathDamageImpulseConversion) / PrimitiveComponent->GetMass(),
				0.f, MaxPhysImpulse);
			PrimitiveComponent->AddImpulseAtLocation(ImpulseDirection * ImpulseStrength, ImpulseLocation);
			return true;
		}
	}
	return false;
}

bool ABaseEnemy_CPP::ApplyTempRadialImpulseToBone(float Damage, UPrimitiveComponent* Component, FName Bone, FVector ImpulseLocation, FVector ImpulseDirection)
{
	ShouldPhysReturn = true;

	USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Component);
	ACharacter* Character = Cast<ACharacter>(Component->GetOwner());
	float ImpulseStrength = 0.f;
	if (SkeletalMesh)
	{
		if (SkeletalMeshReference == SkeletalMesh)
		{
			float impulseScale = OnDeathDamageImpulseConversion;

			ImpulseStrength = FMath::Clamp(
				(Damage * impulseScale) / SkeletalMesh->GetMass(),
				0.f, MaxPhysImpulse);

			// Apply impulse to bone
			if (!bPaused)
			{
				if (!SkeletalMeshReference->IsAnySimulatingPhysics())
					SkeletalMesh->SetAllBodiesBelowSimulatePhysics(SimulatePhysicsFromBone, true, true);

				SkeletalMesh->AccumulateAllBodiesBelowPhysicsBlendWeight(SimulatePhysicsFromBone, 1.f, false);

			}
			else
			{
				if (!bApplyImpulseWhenPaused)
					return false;
			}
			SkeletalMesh->AddRadialImpulse(ImpulseLocation, FMath::Abs((ImpulseLocation - SkeletalMesh->GetComponentLocation()).Size() * 2.f), ImpulseStrength, ERadialImpulseFalloff::RIF_Constant, true);
			return true;

		}
	}
	else
	{
		if (SkeletalMeshReference == Component)
		{
			ImpulseStrength = FMath::Clamp(
				(Damage * OnDeathDamageImpulseConversion) / Component->GetMass(),
				0.f, MaxPhysImpulse);
			Component->AddRadialImpulse(ImpulseLocation, FMath::Abs((ImpulseLocation - Component->GetComponentLocation()).Size() * 2.f), ImpulseStrength, ERadialImpulseFalloff::RIF_Constant, true);
			return true;
		}
	}
	return false;
}
