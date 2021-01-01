// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseProjectile.h"
#include "InterfaceRecieveDamage.h"
#include "Camera/CameraComponent.h" 
#include "GameFramework/SpringArmComponent.h"
#include "PlayerCharacter_CPP.generated.h"

#define POST_PROCESS_LENS_DISTORTION_INDEX 1
#define MESH_POST_RELOAD_INTERP_TOLERANCE .1
#define MESH_POST_RELOAD_IS_RELOADING_TOLERANCE 6.0f
#define WEAPON_RELOAD_VISUAL_FACTOR 2.5
#define COLLISION_CHANNEL_INTERACTABLE ECC_GameTraceChannel2

#define PLAYER_LOCATION_GROUND_OFFSET 70
#define SPHERE_TRACE_END_Z_OFFSET 100
#define SPHERE_TRACE_RADIUS 2.5

#define SET_GRAVITY_SCALE_RATE .05

#define ENEMY_TAG "Enemy"

#define GUN_SOCKET_NAME "GunSocket"


UCLASS()
class DREAMREALM_API APlayerCharacter_CPP : public ACharacter, public IInterfaceRecieveDamage
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter_CPP();

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	TSubclassOf<class UCameraShake> WeaponRecoilCameraShakeClass;
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	TSubclassOf<class AGrappleBelt> GrappleBeltClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MovementAbilities")
	// Maximum time between pressing a movement key twice to dodge in that direction (if double tap dodge is enabled)
	float DodgeInputTimeout = .25f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MovementAbilities")
	// Time player must wait before using dodge ability again 
	float DodgeAbilityTimeout = .5f;

	/* WEAPON VARIABLES */
	UPROPERTY(BlueprintReadWrite)
	// Weapons the player has equipped for this mission
	TArray<class AWeapon_CPP*> PlayerWeapons;
	UPROPERTY(BlueprintReadWrite)
	// Current weapon the player has selected 
	AWeapon_CPP* SelectedWeapon;
	// Points to a gun if the current weapon the player has selected is a gun
	class ABaseGun* SelectedGun;

	UPROPERTY(BlueprintReadWrite)
	// Intensity of the post process lens distortion applied to player's camera
	float PPLensDistortionCurrentIntensity = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	// Intensity of the chromatic aberration applied to player's camera
	float ChromaticAbberationCurrentIntensity = 0.0f;
	
protected:
	// Pointer to the Game Instance. Set in BeginPlay 	
	class UDreamRealmGameInstance* DreamRealmGameInstance;

	// Set in BeginPlay()
	APlayerCameraManager* CameraManager;

	// Set in BeginPlay()
	UPROPERTY(BlueprintReadWrite)
	class UPlayerCharacter_AnimInstance_CPP* AnimationInstance;

	// Reference to dynamic PostProcessLensDistortion material
	UMaterialInstanceDynamic* PostProcessLensDistortion;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* LegSkeletalMesh;

	/* TEST SETTINGS */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AWeapon_CPP> FirstTestWeapon;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AWeapon_CPP> SecondTestWeapon;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AWeapon_CPP> ThirdTestWeapon;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AWeapon_CPP> FourthTestWeapon;

	/* BASIC SETTINGS */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float Health = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float MaxHealth = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	// Determines speed at which player's mesh comes out of reload sequence
	float WeaponReloadOutInterpSpeed = 10.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	UAnimSequence* KickAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float KickDamage = 25.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float KickCooldown = .4f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float KickImpulse = 100000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	USoundCue* KickSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float KickHeadTiltInterpSpeed =	14.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float SlideHeadTiltInterpSpeed = 7.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float DefaultHeadTiltBackInterpSpeed = 5.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	FVector KickTraceStartPoint = FVector(0, -20, 1);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	// How long will enemy remain as ragdoll after being kicked
	float KickRagdollTimer = 2.75f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float KickTimeUntilTiltBack = .25f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float KickHeadTilt = 10.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float KickSphereTraceRadius = 200.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	// Time between subsequent calls to PlayerPhysicalBalance()
	float BalanceSphereTraceRate = 0.033333;

	/* MOVEMENT SETTINGS*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MaxStamina = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	TSubclassOf<class UCameraShake> LeftCameraShake;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	TSubclassOf<UCameraShake> RightCameraShake;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float CrouchSpeed = 250.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float GravityScaleDefault = 2.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float GravityScaleFalling = 8.0f;

	/* DAMAGE EFFECT SETTINGS */
	UPROPERTY(EditDefaultsOnly)
	float PPLensDistortionBlendRate = .05f;
	UPROPERTY(EditDefaultsOnly)
	float ChromaticAbberationBlendRate = .05f;
	UPROPERTY(EditDefaultsOnly)
	float PPLensDistortionDamageMultiplier = .02f;
	UPROPERTY(EditDefaultsOnly)
	float ChromaticAberrationDamageMultiplier = 2.0f;
	UPROPERTY(EditDefaultsOnly)
	float ChromaticAberrationDefault = 1.0f;

	/* HEAD TILT SETTINGS */
	float CurrentHeadTiltInterpSpeed = 0.0f;
	// Desired head tilt amount in degrees
	float DesiredHeadTilt = 0.0f;
	bool ShouldTilt;
	bool ShouldTiltBack;

	/* Variables used to line trace for items in the game world. If an item is hit, 
	*  Post processing is applied to the item by enabling CustomDepth pass on the item's
	*  mesh to give it an outline
	*/
	FVector ItemTickLineTraceStart;
	FVector ItemTickLineTraceEnd;
	FHitResult ItemTraceResult;

	// Keeps track of when behavior to disable post processing highlight around items should be executed
	bool DisableItemCustomRenderOnNextTrace;
	UPrimitiveComponent* ItemTraceActorPrimitiveCompnent;

	// Keeps track of when the mouse is pressed down so that semi-automatic weaponry can be fired
	bool FireButtonHasBeenPressed = false;
	bool CanFire = false;
	FTimerHandle KickTiltHeadBackTimerHandle;
	FTimerHandle KickEndTimerHandle;
	float KickCooldownTimer = 0.0f;

	bool IsReloading = false;
	// Time over which mesh is lerped to the location at which reload will take place (somewhere behind camera)
	float InitialReloadTime = 0.0f;
	// Total time it will take to reload the current weapon (set by weapon before reload)
	float TotalReloadTime = 0.0f;
	// Is the current reload an intermediate reload?
	bool IsIntermediateReload = false;
	// Time over which mesh is lerped to the location at which intermediate reload will take place
	//float InitialIntermediateReloadTime = 0.0f;
	// Total time it will take to perform intermediate reload of current weapon
	//float TotalIntermediateReloadTime = 0.0f;

	bool IsFinishingReloadInterp = false;

	bool IsSwitchingWeaponOut = false;
	bool IsSwitchingWeaponIn = false;

	// Index of the weapon that is being switched to
	int WeaponSwitchIndex = 0;

	// Reference to the timer that sets off a line trace every X seconds to trace for items 
	FTimerHandle ItemTraceTimerHandle;
	FTimerHandle ReloadFinishedTimerHandle;

	/* RECOIL VARIABLES */
	// Current recoil pitch input 
	float CurrentRecoilHorizontal = 0.0f;
	// Current recoil yaw input
	float CurrentRecoilVertical = 0.0f;
	// Time over which recoil will be added
	float CurrentRecoilTime = 0.0f;
	// Current distance the weapon has been raised while simulating recoil
	float CurrentMeshRaiseDistance;
	UPROPERTY(BlueprintReadWrite)
	bool InVault;
	UPROPERTY(BlueprintReadWrite)
	bool InClimb;

	/* DOUBLE TAP DODGE VARIABLES*/
	// Should dodge occur if user double taps movement key
	bool ShouldDoubleTapDodge = true;
	UPROPERTY(BlueprintReadWrite)
	int ForwardDodgeSequence = 0;
	UPROPERTY(BlueprintReadWrite)
	int BackwardDodgeSequence = 0;
	UPROPERTY(BlueprintReadWrite)
	int RightwardDodgeSequence = 0;
	UPROPERTY(BlueprintReadWrite)
	int LeftwardDodgeSequence = 0;
	float ForwardDodgeInputTimer = 0.0f;
	float BackwardDodgeInputTimer = 0.0f;
	float RightwardDodgeInputTimer = 0.0f;
	float LeftwardDodgeInputTimer = 0.0f;

	/* MOVEMENT VARIABLES */
	UPROPERTY(BlueprintReadWrite)
	float Stamina = 100.0f;
	UPROPERTY(BlueprintReadWrite)
	// Time remaining on dodge ability cooldown
	float DodgeAbilityTimeoutCurrent = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	bool InCrouching;
	UPROPERTY(BlueprintReadWrite)
	bool InSlide;
	UPROPERTY(BlueprintReadWrite)
	bool CanForward;
	UPROPERTY(BlueprintReadWrite)
	bool CanForwardSprint;
	UPROPERTY(BlueprintReadWrite)
	bool IsRunning;

	// Remaining time over which the player's mesh will be interpolated to its weapon switch position
	float CurrentWeaponSwitchOutTime = 0.0f;
	
	/**
	 * Speed multiplier affecting rate at which player's mesh will be interpolated back to its original position
	 * after switching weapons
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	float WeaponSwitchInInterpSpeed = 10.0f;
	
	// Used for physical recoil simulation
	FVector MeshOriginalRelativeLocation;

	/**
	 * Location value that is added to the player's mesh to when switching weapons
	 * This could be made a USceneCompnent so that it can be seen in blueprint
	 * where the mesh will move to, but this is slightly more performant
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	FVector WeaponSwitchMeshMoveDistance;
	FVector CurrentWeaponSwitchMoveDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	FVector WeaponReloadMeshMoveDistance;
	FVector CurrentReloadMoveDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	float TimeBetweenItemTrace = .05f;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	float TickLineTraceDistance = 250.0f;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraSpringArmComponent;

	UPROPERTY(BlueprintReadWrite)
	class AGrappleBelt* GrappleBelt;

	/* GRAPPLE BELT VARIABLES */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* GrappleBeltSpawnLocation;
	UPROPERTY(BlueprintReadWrite, Category = "GrappleBelt")
	// Is the player swinging with grapple hook
	bool IsSwinging = false;

	/* VISUAL TIMER VARIABLES */
	FTimerHandle BalanceSphereTraceTimerHandle;
	FTimerHandle SetGravityScaleTimerHandle;

	UFUNCTION(BlueprintCallable)
	float SubtractStamina(float StaminaLoss);

	UFUNCTION()
	void TraceForItem();

	UFUNCTION()
	void Kick();

	UFUNCTION()
	// Sets leg mesh visibility to false after kick has ended
	void KickEnd();

	UFUNCTION(BlueprintCallable)
	void SetHeadTiltInterpSpeed(float Speed);

	UFUNCTION(BlueprintCallable)
	void TiltHeadAddDegres(float Degrees);

	UFUNCTION(BlueprintCallable)
	void TiltHeadSubtractDegrees(float Degrees);

	UFUNCTION(BlueprintCallable)
	void TiltHeadBack();

	UFUNCTION()
	void FireWeapon(float PrimaryFireAxis);

	// Enable special ability of current weapon (if it has one)
	void EnableWeaponSpecial();

	// Disable special ability of current weapon (if it has one)
	void DisableWeaponSpecial();

	UFUNCTION()
	void BeginReloadAnimation();

	UFUNCTION(BlueprintImplementableEvent)
	/**
	* Update text on player UI displaying ammo counts
	*/
	void UpdateAmmoUI(int AmmoInClip, int TotalAmmo);

	UFUNCTION(BlueprintCallable)
	/**
	* Puts dodge ability on cooldown
	*/
	void SetDodgeAbilityCooldown();

	/**
	* Plays finishing reload animation
	*/
	void FinishReload();

	UFUNCTION(BlueprintImplementableEvent)
	/**
	 * Sets reloading to false on this character's animation instance
	 */
	void AnimationInstanceSetReloadingFalse();

	UFUNCTION(BlueprintImplementableEvent)
	/**
	 * Updates UI element showing dodge cooldown
	 */
	void UpdateDodgeCooldownTimer();

	UFUNCTION(BlueprintImplementableEvent)
	/**
	* Hides UI element showing dodge cooldown
	*/
	void HideDodgeCooldownTimer();

	UFUNCTION(BlueprintImplementableEvent)
	/**
	* Pauses game and brings up pause menu
	*/
	void TogglePauseGame();

	UFUNCTION(BlueprintImplementableEvent)
	/**
	* Initiates dodge event
	*/
	void InitiateDodgeEvent();

	/* GRAPLE METHODS */
	/**
	 * Toggles between calling ActivateGrappleBelt() and UnhookGrappleBelt()
	 */
	void GrappleBeltToggle();

	UFUNCTION(BlueprintCallable)
	/**
	 * Fires grapple belt
	 */
	void ActivateGrappleBelt();

	UFUNCTION(BlueprintImplementableEvent)
	void ActivateGrappleBelt_BP();
 
	UFUNCTION(BlueprintCallable)
	/**
	 * Unhook grapple belt
	 */
	void UnhookGrappleBelt();

	UFUNCTION(BlueprintImplementableEvent)
	void UnhookGrappleBelt_BP();
	
	UFUNCTION(BlueprintImplementableEvent)
	class UCapsuleComponent* GetVaultCapsule_BP();

	UFUNCTION(BlueprintImplementableEvent)
	void StopRun_BP();

	/**
	 * Sets timers that are meant to loop indefinitely 
	 */
	void SetLoopingTimers();

	void PlayerPhysicalBalance();

	void SetTickVariables(); 

	void UpdateCameraLocation();

	// Set greater gravity scale when falling to make jumping feel less floaty
	void UpdateGravityScale();

	// Spawn player weapons at start of level
	void SpawnPlayerWeapons();

	/* WEAPON RECOIL METHODS */
	/**
	 * Plays camera shake animation on player's camera
	 */
	void BeginWeaponRecoilCameraShake();

	/**
	 * Adds to CurrentRecoilTime
	 */
	void AddWeaponRecoil();

	/* WEAPON SWITCH METHODS*/
	/**
	 * Initiates weapon switching animation
	 */
	void BeginSwitchWeaponSequence(int WeaponIndex);

	/**
	 * Points CurrentWeapon to the weapon at index WeaponIndex 
	 */
	void EndSwitchWeaponSequence();

	/**
	 * To be bound to OnMontageEnd delegate for when initial weapon switching animation ends
	 */
	//void OnWeaponSwitchAnimationEnded(UAnimMontage* Montage, bool bInterrupted);

	/* METHODS CALLED IN TICK */
	/**
	 * Handles the increasing and decreasing of physical recoil after firing
	 */
	void HandlePhysicalRecoil(float DeltaTime);

	/**
	 * Handles visual aspect of switching weapon out
	 */
	void HandleWeaponSwitchOut(float DeltaTime);

	/**
	 * Handles visual aspect of switching weapon out
	 */
	void HandleWeaponSwitchIn(float DeltaTime);

	/**
	 * Uses interpolation to position mesh correctly during reload sequence
	 */
	void HandleWeaponReload(float DeltaTime);
	
	/**
	 * These events detect when movement key has been double tapped 
	 * so that a dodge event can be initiated
	 */
	void HandleForwardBackwardDodge(float ForwardInput);
	void HandleRightwardLeftwardDodge(float RightwardInput);
	void HandleDodgeInputTimers(float DeltaTime);

	/**
	 * Handles interpolation of head tilt
	 */
	void HandleHeadTilt(float DeltaTime);

public:	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayHitmarker();

	UFUNCTION(BlueprintCallable)
	/**
	 * Sets intensity of damage effects on post process material and variables
	 */
	void SetDamageEffectIntensity();

	UFUNCTION(BlueprintCallable)
	/**
	 * Sets variables for intensity of damage effects by multiplying damage by a scaling factor
	 */
	void CalculateDamageEffectIntensity(float Damage);

	UFUNCTION(BlueprintCallable)
	/**
	 * Calls BeginReload on SelectedWeapon
	 */
	 void BeginReload();

	UFUNCTION(BlueprintCallable)
	/**
	 * Returns MaxHealth
	 */
	float GetMaxHealth();

	UFUNCTION(BlueprintCallable)
	/**
	* Returns Health
	*/
	float GetCurrentHealth();

	UFUNCTION(BlueprintCallable)
	float GetStamina();

	UFUNCTION(BlueprintCallable)
	float GetMaxStamina();
};
