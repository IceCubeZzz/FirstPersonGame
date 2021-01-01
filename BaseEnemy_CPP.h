// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapon_CPP.h"
#include "InterfaceRecieveDamage.h"
#include "BaseAIController_CPP.h"
#include "BaseEnemy_CPP.generated.h"

UCLASS()
class DREAMREALM_API ABaseEnemy_CPP : public ACharacter, public IInterfaceRecieveDamage
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseEnemy_CPP();

	// Class of weapon 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AWeapon_CPP> WeaponType;

	// This variable should be set in the deriving blueprint class
	UPROPERTY(BlueprintReadOnly)
	AWeapon_CPP* Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PhysReactions")
	// Last applied damage is multiplied by this value on death to determine the impulse to apply to the mesh
	float OnDeathDamageImpulseConversion = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PhysReactions")
	float OnBodyPartDetachDamageImpulseConversion = 2.0f;

	/**
	* Multiplier for converting damage to impulse.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PhysReactions")
	float OnHitDamageImpulseConversion = 1.0f;

	/**
	* Multiplier for converting damage to impulse per bones. 
	* Replace ImpulseFromDamageMultiplier
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PhysReactions|SkeletalMesh")
	TMap<FName, float> ImpulseMultiplierPerBones; 

	UPROPERTY(BlueprintReadWrite, Category = Config)
	bool IsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PhysReactions")
	float MaxPhysImpulse = 200.0f;

	/**
	* All bodies below this bone (inclusive) will simulate physics.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PhysReactions")
	FName SimulatePhysicsFromBone = "spine_02";

	/**
	* The speed at which PhysicsBlendWeight tends to 0. Where 0 is the lack of simulation, 1 is a full simulation.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PhysReactions")
	float SpeedExitingPhysicsSimulations = 2.5f;

	UPROPERTY()
	bool bPaused = false;

	UPROPERTY()
	bool bApplyImpulseWhenPaused = false;

	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshReference;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	class UBehaviorTree* BehaviorTree;


protected:
	// The enemy's dynamic material
	UMaterialInstanceDynamic* DynamicMaterial;

	// Should blend back to animation state (instead of simulating physics state)
	bool ShouldPhysReturn = true;

	float RagdollDurationTimer = 0.0f;

	FVector LastHitDirection;
	FVector LastHitLocation;
	float LastHitDamage;
	FName LastHitBoneName;

	float PreviousHealth;

	UPROPERTY(BlueprintReadWrite)
	float CurrentHealth = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadWrite)
	float CurrentDamageMultiplier = 1.0f;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class URagdollComponent* RagdollComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * Used to tell the mesh material that the enemy took damage
	 * so that a white flashing effect can be utilized
	 */
	UFUNCTION(BlueprintCallable)
	void TookDamage();

	UFUNCTION(BlueprintCallable)
	virtual void SetupWeapon(AWeapon_CPP* NewWeapon);

	UFUNCTION(BlueprintImplementableEvent)
	float ApplyDamage(float Damage, float CurrentTotalHealth, float TotalDamageMultiplier, FVector HitLocation, FName BoneName, FVector HitFromDirection, UPrimitiveComponent* FHitComponent, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable)
	void OnBoneBreak(FName BoneName, USkeletalMeshComponent* SkeletalMesh);

	void PhysBlendReturn(float DeltaTime);

	void CheckIfDead();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*UFUNCTION(BlueprintCallable)
	void ApplyImpulseToBrokenPart(AActor* BodyPart, FName BoneName);*/

	UFUNCTION()
	void ApplyImpulseToBone(FVector Impulse, FVector Location, FName BoneName);

	UFUNCTION()
	void ApplyImpulseToLastBone(FVector Impulse, FVector Location);

	UFUNCTION()
	bool ApplyTempImpulseToBone(float Damage, UPrimitiveComponent* PrimitiveComponent, FName Bone, FVector ImpulseLocation, FVector ImpulseDirection);

	UFUNCTION()
	bool ApplyTempRadialImpulseToBone(float Damage, UPrimitiveComponent* Component, FName Bone, FVector ImpulseLocation, FVector ImpulseDirection);

	UFUNCTION()
	void OnPointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser);

	UFUNCTION()
	void OnRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, FHitResult HitInfo, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable)
	void StoreLastHitInfo(FVector HitDirection, FVector HitLocation, float HitDamage, FName BoneName);

	UFUNCTION(BlueprintCallable)
	// Should be called by child blueprint when this enemy dies
	void OnDeath();

	URagdollComponent* GetRagdollComponent();

	UFUNCTION(BlueprintImplementableEvent)
	void EnableRagdoll_BP();

	UFUNCTION(BlueprintCallable)
	void EnableRagdoll();

	UFUNCTION(BlueprintImplementableEvent)
	void DisableRagdoll();

	void EnableTempRagdoll(float RagdollDuration);
};
