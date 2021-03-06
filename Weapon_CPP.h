// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnumWeaponType.h"
#include "GameFramework/Actor.h"
#include "PickupableItem_CPP.h"
#include "Sound/SoundCue.h"
#include "Weapon_CPP.generated.h"

UCLASS()
class DREAMREALM_API AWeapon_CPP : public APickupableItem_CPP
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon_CPP();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// This weapon type
	TEnumAsByte<EWeaponType> Weapon = EWeaponType::SilencedPistol;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// Time it takes to reload the weapon (excludes reload animation time)
	float TimeToReload = .4f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// Time it takes for intermediate reload (pumping shotgun, sniper bolt pull, etc.)
	// 0 value indicates that the weapon does not have an intermediate reload 
	float TimeToReloadIntermediate = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundCue* FireSound;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundCue* ReloadSound;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundCue* IntermediateReloadSound;
	UPROPERTY(EditDefaultsOnly)
	// Should be set to true if the weapon can be repeatedly fired by holding down the fire button 
	bool WeaponIsAutomatic = false;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireSoundMultiplier = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float WeaponDamage = 10.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// Time it takes to switch to another weapon from this weapon
	float WeaponSwitchTime = .5f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	float WeaponRecoilCameraShakeScale = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	// Range of possible value added to player's yaw input per second 
	FVector2D WeaponVerticalRecoilRange = FVector2D(7.5f, 10.f);
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	// Range of possible value added to player's yaw input per second 
	FVector2D WeaponHorizontalRecoilRange = FVector2D(-2.5f, 2.5f);
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	float WeaponRecoilRecoverSpeed = 4.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	// Time over which recoil increases after weapon fire event is called
	float WeaponRecoilIncreaseTime = .05f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	// Maximum distance the player's mesh can be interped to after shots are fired (used to further the effect of recoil)
	float WeaponRecoilMoveDistanceMax = 40.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// If true, firing event will take place at the location of fire socket. If false, they will take place at player eye height.
	bool ShouldFireFromSocket = false;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool WeaponCanAim = false;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// Time it takes to fully aim with the weapon
	float WeaponAimTime = .25f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// How fast weapon will move into place while aiming. Currently unused as smoothstep is being used in place of interp
	float WeaponAimLocationInterpSpeed = 6.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// How fast weapon will rotate while aiming. Weapon must be rotated because it is not pointing directly forwards while it is in the player's hands
	float WeaponAimRotationInterpSpeed = 12.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// Dictates additive to player local mesh location when player is aiming (if the weapon is aimable)
	FVector	AimOffsetLocation = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// Dictates additive to player local mesh rotation when player is aiming 
	FRotator AimOffsetRotation = FRotator::ZeroRotator;

	/* AMMO SETTINGS */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	/**
		* Maximum ammo in a clip
		* -1 signifies that the gun never has to be reloaded.
		*/
	int MaxClipAmmo = -1;
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	/**
		* Maximum backup ammo that can be held for this gun
		* -1 signifies that the gun has no maximum ammo count.
		*/
	int MaxBackupAmmo = -1;
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int StartingAmmo = MAX_int32;

	/**
	 * Because some gun meshes are static meshes, while others are skeletal meshes,
	 * each gun object contains both a static mesh component and a skeletal mesh component;
	 * Both components are initialized, but only one should have a mesh assigned
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* WeaponMeshStatic;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMeshSkeletal;

	UPROPERTY(EditDefaultsOnly)
	UAnimSequenceBase* WeaponFireAnimation;

	UPROPERTY(EditDefaultsOnly)
	UAnimSequenceBase* PlayerMeshFireAnimation;

	// Ammo in current magazine
	int CurrentAmmoClip;
	// Current total ammo
	int CurrentAmmoTotal;

	// Currently unused. Changing mesh location directly instead of using an animation 
	//UPROPERTY(EditDefaultsOnly)
	UAnimSequenceBase* PlayerMeshReloadAnimation;

	FVector ProjectileNonSocketSpawnPoint;

	// Only used for guns that have an intermediate reload time value 
	bool IntermediateReloadCompleted = true;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Used to manipulate camera shake scale from outside this class
	float WeaponRecoilCameraShakeScaleMultiplier = 1.0f;

	// Used to manipulate the recoil move distance from outside this class
	float WeaponRecoilMoveDistanceMaxMultiplier = 1.0f;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	/**
	 * Fires gun and returns true if gun firing is not on cooldown and projectile was successfully fired
	 */
	virtual bool Fire();

	virtual bool BeginReloadAnimation();

	virtual void ActivateSpecialAbility();

	UFUNCTION(BlueprintCallable)
	virtual void SetLocationToFireTowards(FVector NewLocation);

	virtual void FireTowardsLocation();

	UFUNCTION(BlueprintCallable)
	void SetAimOffsetLocation(FVector Offset);

	FVector GetAimOffsetLocation();

	FRotator GetAimOffsetRotation();

	bool GetWeaponCanAim();

	/**
	 * Initiates post reload animation logic (for example, reloading the current clip)
	 */
	virtual void BeginReload();
	
	virtual void PlayGunFireAnimation();

	UAnimSequenceBase* GetPlayerMeshFireAnimation();

	UAnimSequenceBase* GetPlayerMeshReloadAnimation();

	virtual bool GetWeaponIsAutomatic();

	/**
	* Returns CurrentAmmoClip. If value of -1 is returned, the weapon does not use clips.
	* A melee weapon would always return -1
	*/
	virtual int GetCurrentAmmo();

	/**
	* Returns CurrentAmmoTotal. If a value of -1 is returned, the weapon has infinite ammo
	* A melee weapon would always return -1
	*/
	virtual int GetTotalAmmo();

	virtual int GetMaxClipAmmo();

	/**
	* Returns the time it takes this weapon to reload. If the weapon never needs to be reloaded,
	* this time is irrelevant
	*/
	float GetTimeToReload();

	float GetTimeToReloadIntermediate();

	bool GetIntermediateReloadCompleted();

	void SetIntermediateReloadCompleted(bool Completed);

	USoundCue* GetReloadSound();

	USoundCue* GetIntermediateReloadSound();

	float GetWeaponDamage();

	float GetWeaponSwitchTime();

	float GetWeaponAimTime();

	float GetWeaponAimLocationInterpSpeed();

	float GetWeaponAimRotationInterpSpeed();
	
	void SetWeaponRecoilCameraShakeScale(float Scale);

	float GetWeaponRecoilCameraShakeScale();

	FVector2D GetWeaponVerticalRecoilPerSecond();

	FVector2D GetWeaponHorizontalRecoilPerSecond();

	float GetWeaponRecoilRecoverSpeed();

	float GetWeaponRecoilIncreaseTime();

	void SetWeaponRecoilMoveDistanceMax(float Distance);

	float GetWeaponRecoilMoveDistanceMax();

	EWeaponType GetWeaponType();

	UFUNCTION(BlueprintCallable)
	void HideWeaponMesh();

	UFUNCTION(BlueprintCallable)
	void ShowWeaponMesh();

	UFUNCTION(BlueprintCallable)
	UStaticMeshComponent* GetWeaponStaticMeshComponent();

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetWeaponSkeletalMeshComponent();

	/*void SetProjectileNonSocketSpawnPoint(FVector SpawnPoint);

	void SetShouldFireFromSocket(bool FireFromSocket);

	bool GetShouldFireFromSocket();*/
};
