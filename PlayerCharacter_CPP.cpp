// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#include "PlayerCharacter_CPP.h"
#include "DreamRealmGameInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "InterfaceUsable.h"
#include "Weapon_CPP.h"
#include "BaseGun.h"
#include "WeaponRecoilCameraShake.h"
#include "GrappleBelt.h"
#include "PlayerCharacter_AnimInstance_CPP.h"
#include "Camera/CameraShake.h"
#include "DreamRealmGameInstance.h"
#include "Components/CapsuleComponent.h" 

// Sets default values
APlayerCharacter_CPP::APlayerCharacter_CPP()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("FirstPersonCameraSpringArmComponent");
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>("FirstPersonCamera");
	GrappleBeltSpawnLocation = CreateDefaultSubobject<USceneComponent>("Grapple Belt Location");
	LegSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Leg Skeletal Mesh");

	// Setup component attachment order
	FirstPersonCamera->SetupAttachment(GetRootComponent());
	CameraSpringArmComponent->SetupAttachment(FirstPersonCamera);
	GetMesh()->SetupAttachment(CameraSpringArmComponent);
	GrappleBeltSpawnLocation->SetupAttachment(GetRootComponent());
	LegSkeletalMesh->SetupAttachment(CameraSpringArmComponent);

	/*FirstPersonCamera->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	CameraSpringArmComponent->AttachToComponent(FirstPersonCamera, FAttachmentTransformRules::KeepRelativeTransform);
	GetMesh()->AttachToComponent(CameraSpringArmComponent, FAttachmentTransformRules::KeepRelativeTransform);
	GrappleBeltSpawnLocation->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);*/
}

// Called when the game starts or when spawned
void APlayerCharacter_CPP::BeginPlay()
{
	Super::BeginPlay();	

	DreamRealmGameInstance = Cast<UDreamRealmGameInstance>(GetGameInstance());

	GetWorld()->GetTimerManager().SetTimer(ItemTraceTimerHandle, this, &APlayerCharacter_CPP::TraceForItem, TimeBetweenItemTrace, true);

	// Set reference to APlayerCameraManager
	CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	// Set reference to the player's AnimInstance
	AnimationInstance = Cast<UPlayerCharacter_AnimInstance_CPP>(GetMesh()->GetAnimInstance());

	/**
	 * Convert the PostProcessLensDistortion material on the player's camera to
	 * a dynamic material so that its properties can be changed during runtime
	 */
	FPostProcessSettings* CameraPPSettings = &(FirstPersonCamera->PostProcessSettings);
	if (CameraPPSettings->WeightedBlendables.Array.Num() > 0)
	{
		auto* Mat = Cast<UMaterialInstance>(CameraPPSettings->WeightedBlendables.Array[POST_PROCESS_LENS_DISTORTION_INDEX].Object);
		if (Mat != nullptr)
		{
			PostProcessLensDistortion = UMaterialInstanceDynamic::Create(Mat, nullptr);
			CameraPPSettings->WeightedBlendables.Array[POST_PROCESS_LENS_DISTORTION_INDEX].Object = PostProcessLensDistortion;
		}
	}

	/**
	 * Store the original location of mesh so that we know where to interp the mesh back to 
	 * after playing recoil effects
	 */
	if (ensure(GetMesh()))
	{
		MeshOriginalRelativeLocation = GetMesh()->GetRelativeLocation();
	}

	// Create grapple belt
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	GrappleBelt = GetWorld()->SpawnActor<AGrappleBelt>(GrappleBeltClass, GrappleBeltSpawnLocation->GetComponentTransform(), SpawnParams);

	if (GrappleBelt)
	{
		GrappleBelt->AttachToComponent(GrappleBeltSpawnLocation, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	/* FOR TESTING PURPOSES ONLY */

	UDreamRealmGameInstance* GameInstance = Cast<UDreamRealmGameInstance>(GetGameInstance());

	GameInstance->PlayerSelectedWeapons.Add(FirstTestWeapon);
	GameInstance->PlayerSelectedWeapons.Add(SecondTestWeapon);
	GameInstance->PlayerSelectedWeapons.Add(ThirdTestWeapon);
	GameInstance->PlayerSelectedWeapons.Add(FourthTestWeapon);

	/* ************************* */

	SetLoopingTimers();
	SpawnPlayerWeapons();
	SelectedWeapon = PlayerWeapons[0];
}

float APlayerCharacter_CPP::SubtractStamina(float StaminaLoss)
{
	Stamina -= StaminaLoss;
	return Stamina;
}

void APlayerCharacter_CPP::TraceForItem()
{
	if (ensure(CameraManager))
	{
		if (DisableItemCustomRenderOnNextTrace)
		{
			if (ItemTraceActorPrimitiveCompnent)
			{
				ItemTraceActorPrimitiveCompnent->SetRenderCustomDepth(false);
			}

			DisableItemCustomRenderOnNextTrace = false;
		}
		
		ItemTickLineTraceStart = CameraManager->GetCameraLocation();
		ItemTickLineTraceEnd = ItemTickLineTraceStart + CameraManager->GetActorForwardVector() * TickLineTraceDistance;

		bool Hit = GetWorld()->LineTraceSingleByChannel(
			ItemTraceResult,
			ItemTickLineTraceStart,
			ItemTickLineTraceEnd,
			ECollisionChannel::COLLISION_CHANNEL_INTERACTABLE);
			
		bool ImplementsInterface = false;
		if (Hit && ItemTraceResult.GetActor())
		{
			ImplementsInterface = ItemTraceResult.GetActor()->GetClass()->ImplementsInterface(UInterfaceUsable::StaticClass());
		}

		if (ImplementsInterface)
		{
			DisableItemCustomRenderOnNextTrace = true;

			ItemTraceActorPrimitiveCompnent = Cast<UPrimitiveComponent>(ItemTraceResult.GetActor()->GetComponentByClass(UPrimitiveComponent::StaticClass()));
			ItemTraceActorPrimitiveCompnent->SetRenderCustomDepth(true);
		}
	}
}

void APlayerCharacter_CPP::Kick()
{
	if (KickAnimation && KickCooldownTimer <= 0)
	{
		LegSkeletalMesh->SetVisibility(true);

		if(KickSound)
			UGameplayStatics::PlaySound2D(this, KickSound);

		LegSkeletalMesh->PlayAnimation(KickAnimation, false);
		SetHeadTiltInterpSpeed(KickHeadTiltInterpSpeed);
		TiltHeadAddDegres(KickHeadTilt);
		GetWorld()->GetTimerManager().SetTimer(KickEndTimerHandle, this, &APlayerCharacter_CPP::KickEnd, KickAnimation->GetPlayLength(), false);
		
		// If head is already being tilted back, there is no need to subtract head tilt degrees. 
		if (!ShouldTiltBack)
		{
			FTimerDelegate HeadTiltBackTimerDelegate;
			HeadTiltBackTimerDelegate.BindUFunction(this, FName("TiltHeadSubtractDegrees"), KickHeadTilt);
			GetWorld()->GetTimerManager().SetTimer(KickTiltHeadBackTimerHandle, HeadTiltBackTimerDelegate, KickTimeUntilTiltBack, false);
		}
		
		// Use sphere sweep to detect enemies hit by kick
		FCollisionShape Sphere = FCollisionShape::MakeSphere(KickSphereTraceRadius);
		TArray<FHitResult> OutResults;
		GetWorld()->SweepMultiByChannel(OutResults, LegSkeletalMesh->GetComponentLocation() + FVector(0, KickTraceStartPoint.Y, KickTraceStartPoint.Z), LegSkeletalMesh->GetComponentLocation() + FVector(0, -KickTraceStartPoint.Y, KickTraceStartPoint.Z),
			FQuat::Identity, ECollisionChannel::ECC_Pawn, Sphere);

		for (FHitResult HitResult : OutResults)
		{
			// If the actor has this tag, then they are an enemy that has been hit by the kick
			if (HitResult.Actor->ActorHasTag(ENEMY_TAG))
			{
				ABaseEnemy_CPP* Enemy = Cast<ABaseEnemy_CPP>(HitResult.Actor);

				// Manually set bone name
				HitResult.BoneName = Enemy->GetMesh()->FindClosestBone(HitResult.ImpactPoint, NULL, 0.0f, true);

				Enemy->EnableTempRagdoll(KickRagdollTimer);
				UGameplayStatics::ApplyPointDamage(Enemy, KickDamage, HitResult.ImpactPoint - Enemy->GetActorLocation(), HitResult, this->GetController(), this, nullptr);
				Enemy->ApplyImpulseToBone(KickImpulse * (Enemy->GetActorLocation() - HitResult.ImpactPoint).GetSafeNormal(), HitResult.ImpactPoint, HitResult.BoneName);
				DisplayHitmarker();

				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, HitResult.BoneName.ToString());
			}
		}

		KickCooldownTimer = KickCooldown;
	}
}

void APlayerCharacter_CPP::KickEnd()
{
	GetWorld()->GetTimerManager().ClearTimer(KickEndTimerHandle);

	LegSkeletalMesh->SetVisibility(false);
}

void APlayerCharacter_CPP::SetHeadTiltInterpSpeed(float Speed)
{
	CurrentHeadTiltInterpSpeed = Speed;
}

void APlayerCharacter_CPP::TiltHeadAddDegres(float Degrees)
{
	if (CurrentHeadTiltInterpSpeed <= 0.0f)
	{
		CurrentHeadTiltInterpSpeed = DefaultHeadTiltBackInterpSpeed;
	}

	ShouldTilt = true;
	DesiredHeadTilt += Degrees;
}

void APlayerCharacter_CPP::TiltHeadSubtractDegrees(float Degrees)
{
	if (CurrentHeadTiltInterpSpeed <= 0.0f)
	{
		CurrentHeadTiltInterpSpeed = DefaultHeadTiltBackInterpSpeed;
	}

	ShouldTilt = true;
	DesiredHeadTilt -= Degrees;
}

void APlayerCharacter_CPP::TiltHeadBack()
{
	ShouldTilt = false;
	ShouldTiltBack = true;
	DesiredHeadTilt = 0;
}

void APlayerCharacter_CPP::FireWeapon(float PrimaryFireAxis)
{
	if (PrimaryFireAxis > 0 && SelectedWeapon && !IsReloading)
	{
		// If the selected weapon is semi-automatic, only allow the user to fire each time the fire button is pressed down
		// This means that holding the fire button down will not repeatedly fire the weapon
		if(!SelectedWeapon->GetIsAutomatic())
		{ 
			CanFire = !FireButtonHasBeenPressed ? true : false;
		}
		else
		{ 
			CanFire = true;
		}

		FireButtonHasBeenPressed = true;
		
		// Ensure the player can fire, and ensure that there is a valid CameraManager reference before firing
		if (CanFire && CameraManager)
		{ 
			//APlayerController* PlayerController = Cast<APlayerController>(GetController());

			// Retrieve the point the player is aiming at
			FVector EyeLocation;
			FRotator EyeRotation;
			CameraManager->GetActorEyesViewPoint(EyeLocation, EyeRotation);
			FVector FireLocation = EyeLocation + (EyeRotation.Vector() * 2000);

			// Fire towards the point the player is aiming at
			SelectedWeapon->SetLocationToFireTowards(FireLocation);
			
			if (SelectedGun && SelectedGun->GetShouldLineTraceTest())
			{
				SelectedGun->SetLineTraceTestStart(EyeLocation + (EyeRotation.Vector() * 25), EyeRotation);
			}

			/*if (SelectedWeapon->GetShouldFireFromSocket())
			{
				SelectedWeapon->SetProjectileNonSocketSpawnPoint(EyeLocation);
			}*/

			if (SelectedWeapon->Fire())
			{
				auto* FireAnimation = SelectedWeapon->GetPlayerMeshFireAnimation();
				if (AnimationInstance && FireAnimation)
				{
					AnimationInstance->PlaySlotAnimationAsDynamicMontage(FireAnimation, "GunAction", .2f, .05f);
				}

				AddWeaponRecoil();
				BeginWeaponRecoilCameraShake();
				UpdateAmmoUI(SelectedWeapon->GetCurrentAmmo(), SelectedWeapon->GetTotalAmmo());
			}
		}
	}
	else if (PrimaryFireAxis <= 0)
	{
		FireButtonHasBeenPressed = false;
	}
}

void APlayerCharacter_CPP::ActivateWeaponSpecial()
{
	/* If aiming down sights is possible with current weapon, then aiming down sights
	 * is the weapon's special ability. 
	 */
	if (SelectedWeapon->GetWeaponCanAim())
	{
		ToggleAim();
	}
	else
	{
		SelectedWeapon->ActivateSpecialAbility();
	}
}

void APlayerCharacter_CPP::ToggleAim()
{
	if (!IsAiming)
	{
		// Move mesh in front of player's view with an offset specified by the weapon
		FVector EyeLocation;
		FRotator EyeRotation;
		CameraManager->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		MeshTargetWorldLocationAiming = EyeLocation + (EyeRotation.Vector() * AimDistanceFromPlayer) + SelectedWeapon->GetAimOffset();

		IsAiming = true;

		if ( !(GetWorld()->GetTimerManager().IsTimerActive(AimingTimerHandle)) )
		{
			GetWorld()->GetTimerManager().SetTimer(AimingTimerHandle, this, &APlayerCharacter_CPP::AimMoveMesh, AimTimerResolution, true);
		}
	}
	else
	{
		IsAiming = false;
	}
}

void APlayerCharacter_CPP::AimMoveMesh()
{
	if (IsAiming)
	{
		CurrentMeshAimDistance = FMath::VInterpTo(CurrentMeshAimDistance, MeshTargetWorldLocationAiming, AimTimerResolution, SelectedWeapon->GetWeaponAimInterpSpeed());
		if (FVector::DistSquared(CurrentMeshAimDistance, MeshTargetWorldLocationAiming) < AimDistanceToleranceSquared)
		{
			GetWorld()->GetTimerManager().ClearTimer(AimingTimerHandle);
		}
	}
	else
	{
		CurrentMeshAimDistance = FMath::VInterpTo(CurrentMeshAimDistance, FVector::ZeroVector, AimTimerResolution, SelectedWeapon->GetWeaponAimInterpSpeed());
		if (FVector::DistSquared(CurrentMeshAimDistance, FVector::ZeroVector) < AimDistanceToleranceSquared)
		{
			GetWorld()->GetTimerManager().ClearTimer(AimingTimerHandle);
		}
	}

	GetMesh()->SetRelativeLocation(MeshOriginalRelativeLocation + CurrentMeshRaiseDistance + FVector(0, 0, CurrentMeshRaiseDistance), false);
}

void APlayerCharacter_CPP::BeginReloadAnimation()
{
	if (!IsReloading && SelectedWeapon)
	{
		if(SelectedWeapon->BeginReloadAnimation())
		{
			auto* ReloadAnimation = SelectedWeapon->GetPlayerMeshReloadAnimation();
			if (AnimationInstance && ReloadAnimation)
			{
				InitialReloadTime = ReloadAnimation->GetPlayLength();
				AnimationInstance->PlaySlotAnimationAsDynamicMontage(ReloadAnimation, "GunAction", 0.1f, 0.0f);
			}
		}
	}
}

// Called every frame
void APlayerCharacter_CPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	HandlePhysicalRecoil(DeltaTime);
	HandleWeaponSwitchOut(DeltaTime);
	HandleWeaponSwitchIn(DeltaTime);

	HandleWeaponReload(DeltaTime);

	HandleHeadTilt(DeltaTime);

	SetTickVariables();

	UpdateCameraLocation();

	if (KickCooldownTimer > 0)
	{
		KickCooldownTimer -= DeltaTime;
	}

	if(PPLensDistortionCurrentIntensity > 0)
	{
		SetDamageEffectIntensity();
		PPLensDistortionCurrentIntensity = FMath::Lerp(PPLensDistortionCurrentIntensity, 0.0f, PPLensDistortionBlendRate);
	}
	if (ChromaticAbberationCurrentIntensity > ChromaticAberrationDefault)
	{
		ChromaticAbberationCurrentIntensity = FMath::Lerp(ChromaticAbberationCurrentIntensity, ChromaticAberrationDefault, ChromaticAbberationBlendRate);
	}
	
	// Tick down dodge ability cooldown
	if (DodgeAbilityTimeoutCurrent > 0)
	{
		UpdateDodgeCooldownTimer();
		DodgeAbilityTimeoutCurrent -= DeltaTime;

		if (DodgeAbilityTimeoutCurrent <= 0)
		{
			HideDodgeCooldownTimer();
		}
	}

	if (GrappleBelt)
	{
		GrappleBelt->HandleSwingOwner(this, DeltaTime);
	}

	if(ShouldDoubleTapDodge)
	{ 
		HandleDodgeInputTimers(DeltaTime);
	}
	
	//MeshTotalRelativeLocationOffset = CurrentMeshRaiseDistance + FVector(0, 0, CurrentMeshRaiseDistance);
}

// Called to bind functionality to input
void APlayerCharacter_CPP::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	InputComponent->BindAction("Kick", IE_Pressed, this, &APlayerCharacter_CPP::Kick);

	InputComponent->BindAxis("Primary Fire", this, &APlayerCharacter_CPP::FireWeapon);
	InputComponent->BindAction("Reload", IE_Pressed, this, &APlayerCharacter_CPP::BeginReload);

	InputComponent->BindAction("PauseGame", IE_Pressed, this, &APlayerCharacter_CPP::TogglePauseGame);

	InputComponent->BindAxis("Move Forward", this, &APlayerCharacter_CPP::HandleForwardBackwardDodge);
	InputComponent->BindAxis("Move Right", this, &APlayerCharacter_CPP::HandleRightwardLeftwardDodge);

	InputComponent->BindAction("Grapple", IE_Pressed, this, &APlayerCharacter_CPP::GrappleBeltToggle);

	InputComponent->BindAction("Secondary Fire", IE_Pressed, this, &APlayerCharacter_CPP::ActivateWeaponSpecial);
	InputComponent->BindAction("Secondary Fire", IE_Released, this, &APlayerCharacter_CPP::ActivateWeaponSpecial);

	DECLARE_DELEGATE_OneParam(FSwitchWeaponInputDelegate, const int);
	InputComponent->BindAction<FSwitchWeaponInputDelegate>("Switch Weapon 1", IE_Pressed, this, &APlayerCharacter_CPP::BeginSwitchWeaponSequence, 0);
	InputComponent->BindAction<FSwitchWeaponInputDelegate>("Switch Weapon 2", IE_Pressed, this, &APlayerCharacter_CPP::BeginSwitchWeaponSequence, 1);
	InputComponent->BindAction<FSwitchWeaponInputDelegate>("Switch Weapon 3", IE_Pressed, this, &APlayerCharacter_CPP::BeginSwitchWeaponSequence, 2);
	InputComponent->BindAction<FSwitchWeaponInputDelegate>("Switch Weapon 4", IE_Pressed, this, &APlayerCharacter_CPP::BeginSwitchWeaponSequence, 3);
}

void APlayerCharacter_CPP::SetDamageEffectIntensity()
{
	if (PostProcessLensDistortion)
	{
		PostProcessLensDistortion->SetScalarParameterValue("Intensity", PPLensDistortionCurrentIntensity);
	}
	if (ensure(FirstPersonCamera))
	{
		FirstPersonCamera->PostProcessSettings.SceneFringeIntensity = ChromaticAbberationCurrentIntensity;
		FirstPersonCamera->PostProcessSettings.bOverride_SceneFringeIntensity = 1;
	}
}

void APlayerCharacter_CPP::CalculateDamageEffectIntensity(float Damage)
{
	PPLensDistortionCurrentIntensity = Damage * PPLensDistortionDamageMultiplier;
	ChromaticAbberationCurrentIntensity = (Damage * ChromaticAberrationDamageMultiplier) + ChromaticAberrationDefault;
}

void APlayerCharacter_CPP::BeginReload()
{
	// Check if weapon has intermediate reloading (pumping shotgun, sniper bolt pull, etc.), and if intermediate reload needs to be performed
	if (SelectedWeapon->GetTimeToReloadIntermediate() > 0.0f && !(SelectedWeapon->GetIntermediateReloadCompleted()))
	{
		// Begin intermediate reload
		GetWorld()->GetTimerManager().SetTimer(ReloadFinishedTimerHandle, this, &APlayerCharacter_CPP::FinishReload, SelectedWeapon->GetTimeToReloadIntermediate(), false);
		InitialReloadTime = SelectedWeapon->GetTimeToReloadIntermediate() / WEAPON_RELOAD_VISUAL_FACTOR;
		TotalReloadTime = SelectedWeapon->GetTimeToReloadIntermediate();
		IsIntermediateReload = true;
	}

	// Reload system using lerping of mesh location
	if (SelectedWeapon && SelectedWeapon->GetTotalAmmo() > 0 && !IsReloading && !IsFinishingReloadInterp && !IsSwitchingWeaponOut && !IsSwitchingWeaponIn)
	{
		GetWorld()->GetTimerManager().SetTimer(ReloadFinishedTimerHandle, this, &APlayerCharacter_CPP::FinishReload, SelectedWeapon->GetTimeToReload(), false);

		/**
		 * Divide the it takes to reload weapon by some number since InitialReloadTime is the 
		 * time it takes for player's mesh to be lerped to the location behind the camera.
		 * While this time should be greater on weapons that have greater reload times,
		 * it should not be equal to the total reload time
		 */
		InitialReloadTime = SelectedWeapon->GetTimeToReload() / WEAPON_RELOAD_VISUAL_FACTOR;
		TotalReloadTime = SelectedWeapon->GetTimeToReload();
		IsIntermediateReload = false;
	}

	// Reload system using animations
	//GetMesh()->SetVisibility(false);
	//if (SelectedWeapon)
	//{
	//	SelectedWeapon->HideWeaponMesh();

	//	SelectedWeapon->BeginReload();
	//	UpdateAmmoUI(SelectedWeapon->GetCurrentAmmo(), SelectedWeapon->GetTotalAmmo());

	//	GetWorld()->GetTimerManager().SetTimer(ReloadFinishedTimerHandle, this, &APlayerCharacter_CPP::FinishReload, SelectedWeapon->GetTimeToReload(), false);
	//}
}

float APlayerCharacter_CPP::GetMaxHealth()
{
	return MaxHealth;
}

float APlayerCharacter_CPP::GetCurrentHealth()
{
	return Health;
}

float APlayerCharacter_CPP::GetStamina()
{
	return Stamina;
}

float APlayerCharacter_CPP::GetMaxStamina()
{
	return MaxStamina;
}

void APlayerCharacter_CPP::SetDodgeAbilityCooldown()
{
	DodgeAbilityTimeoutCurrent = DodgeAbilityTimeout; 
}

void APlayerCharacter_CPP::FinishReload()
{
	GetWorld()->GetTimerManager().ClearTimer(ReloadFinishedTimerHandle);

	SelectedWeapon->ShowWeaponMesh();
	//GetMesh()->SetVisibility(true);

	CurrentReloadMoveDistance = FVector::ZeroVector;
	//IsReloading = false;
	IsFinishingReloadInterp = true;

	if (SelectedWeapon && AnimationInstance)
	{
		//SelectedWeapon->ShowWeaponMesh();
		AnimationInstanceSetReloadingFalse();

		if (!IsIntermediateReload)
		{
			SelectedWeapon->BeginReload();
		}
		else
		{
			SelectedWeapon->SetIntermediateReloadCompleted(true);
		}
		UpdateAmmoUI(SelectedWeapon->GetCurrentAmmo(), SelectedWeapon->GetTotalAmmo());
	}
	InitialReloadTime = 0.0;
	
	// Reload system using lerping of mesh location

	// Reload system using animations
	//auto* ReloadAnimation = SelectedWeapon->GetPlayerMeshReloadAnimation();
	//if (AnimationInstance && ReloadAnimation)
	//{
	//	AnimationInstanceSetReloadingFalse();
	//	/**
	//	 * The animation is played backward from the 2nd to last frame. 
	//	 * There is an AnimNotify on the last frame, so this frame has to be avoided when playing backward
	//	 */
	//	AnimationInstance->PlaySlotAnimationAsDynamicMontage(ReloadAnimation, "GunAction", 0.0f, 0.0f, -1.f, 1, 0.0f, ReloadAnimation->GetTimeAtFrame((ReloadAnimation->GetNumberOfFrames() - 2)));
	//	InitialReloadTime = 0.0;
	//}
}

void APlayerCharacter_CPP::GrappleBeltToggle()
{
	if (GrappleBelt->CanUnhook())
	{
		UnhookGrappleBelt();
	}
	else if (GrappleBelt->CanFire())
	{
		ActivateGrappleBelt();
	}
}

void APlayerCharacter_CPP::ActivateGrappleBelt()
{
	if (GrappleBelt && CameraManager)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		CameraManager->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector AimLocation = EyeLocation + (EyeRotation.Vector() * 500);

		if (GrappleBelt->FireHook(AimLocation))
		{
			ActivateGrappleBelt_BP();
		}
	}
	else
	{
		// Play cooldown noise
	}
}

void APlayerCharacter_CPP::UnhookGrappleBelt()
{
	if (GrappleBelt)
	{
		GrappleBelt->UnHook();
		UnhookGrappleBelt_BP();
	}
}

void APlayerCharacter_CPP::SetLoopingTimers()
{
	GetWorld()->GetTimerManager().SetTimer(BalanceSphereTraceTimerHandle, this, &APlayerCharacter_CPP::PlayerPhysicalBalance, BalanceSphereTraceRate, true, BalanceSphereTraceRate);
	GetWorld()->GetTimerManager().SetTimer(SetGravityScaleTimerHandle, this, &APlayerCharacter_CPP::UpdateGravityScale, SET_GRAVITY_SCALE_RATE, true, SET_GRAVITY_SCALE_RATE);
}

void APlayerCharacter_CPP::PlayerPhysicalBalance()
{
	if (!GetCharacterMovement()->IsFalling() && !InVault && !InClimb)
	{
		FVector CapsuleRightVector = GetVaultCapsule_BP()->GetRightVector();
		FVector ActorBottom = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - PLAYER_LOCATION_GROUND_OFFSET);

		FVector StartLocation;
		FVector EndLocation;
		FHitResult SphereTraceResult;
		bool SuccessfulHit;
		TArray<AActor*> ActorsToIgnore;

		StartLocation = CapsuleRightVector * 10 + ActorBottom;
		EndLocation = FVector(StartLocation.X, StartLocation.Y, StartLocation.Z - SPHERE_TRACE_END_Z_OFFSET);

		SuccessfulHit = UKismetSystemLibrary::SphereTraceSingle(this, StartLocation, EndLocation, SPHERE_TRACE_RADIUS,
			UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None,
			SphereTraceResult, true);

		if (!SuccessfulHit)
		{
			GetWorld()->GetFirstPlayerController()->ClientPlayCameraShake(LeftCameraShake, 1.0f);
		}

		StartLocation = CapsuleRightVector * -10 + ActorBottom;
		EndLocation = FVector(StartLocation.X, StartLocation.Y, StartLocation.Z - SPHERE_TRACE_END_Z_OFFSET);

		SuccessfulHit = UKismetSystemLibrary::SphereTraceSingle(this, StartLocation, EndLocation, SPHERE_TRACE_RADIUS,
			UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None,
			SphereTraceResult, true);

		if (!SuccessfulHit)
		{
			GetWorld()->GetFirstPlayerController()->ClientPlayCameraShake(RightCameraShake, 1.0f);
		}
	}
}

void APlayerCharacter_CPP::SetTickVariables()
{
	if (InCrouching && !InSlide)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
	}

	if (CanForward)
	{
		CanForwardSprint = true;
	}
	else
	{
		CanForwardSprint = false;
		if (IsRunning)
		{
			//CanForwardSprint = false;
			StopRun_BP();
		}
	}
}

void APlayerCharacter_CPP::UpdateCameraLocation()
{

}

void APlayerCharacter_CPP::UpdateGravityScale()
{
	if (GetVelocity().Z < 0)
	{
		GetCharacterMovement()->GravityScale = GravityScaleFalling;
	}
	else
	{
		GetCharacterMovement()->GravityScale = GravityScaleDefault;
	}
}

void APlayerCharacter_CPP::SpawnPlayerWeapons()
{
	UDreamRealmGameInstance* GameInstancePtr = Cast<UDreamRealmGameInstance>(GetGameInstance());
	if (GameInstancePtr)
	{
		for (TSubclassOf<AWeapon_CPP> WeaponToSpawn : GameInstancePtr->PlayerSelectedWeapons)
		{
			if (WeaponToSpawn)
			{
				FTransform SpawnTransform = GetTransform();

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = this;

				AWeapon_CPP* SpawnedWeapon = GetWorld()->SpawnActor<AWeapon_CPP>(WeaponToSpawn, SpawnTransform, SpawnParams);
				SpawnedWeapon->SetActorScale3D(FVector(.5, .5, .5));

				FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules(
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::KeepWorld,
					false);
				
				SpawnedWeapon->AttachToComponent(GetMesh(), AttachmentRules, GUN_SOCKET_NAME);

				PlayerWeapons.Add(SpawnedWeapon);
			}
		}
	}
}

void APlayerCharacter_CPP::BeginWeaponRecoilCameraShake()
{
	if (CameraManager && WeaponRecoilCameraShakeClass)
	{
		float RecoilCameraShakeScale = SelectedWeapon->GetWeaponRecoilCameraShakeScale();

		CameraManager->PlayCameraShake(WeaponRecoilCameraShakeClass, RecoilCameraShakeScale);
	}
}

void APlayerCharacter_CPP::AddWeaponRecoil()
{
	CurrentRecoilTime += SelectedWeapon->GetWeaponRecoilIncreaseTime();
}

void APlayerCharacter_CPP::BeginSwitchWeaponSequence(int WeaponIndex)
{
	// Do not allow player to switch weapon to the same weapon
	if(WeaponIndex != WeaponSwitchIndex && !IsReloading && PlayerWeapons.IsValidIndex(WeaponIndex))
	{ 
		WeaponSwitchIndex = WeaponIndex;

		/**
		 * Note: If weapon is being switched out, more time should not be added to
		 * CurrentWeaponSwitchOutTime if user chooses a different weapon to switch to
		 */
		if(SelectedWeapon && !IsSwitchingWeaponOut)
		{ 
			IsSwitchingWeaponOut = true;

			CurrentWeaponSwitchOutTime = SelectedWeapon->GetWeaponSwitchTime();
		}
		/**
		 * If the user switches weapon while weapon was in switching out phase, 
		 * cancel interpolation of mesh back to its original position by setting IsSwitchingWeaponOut to false
		 */ 
		else if (SelectedWeapon && IsSwitchingWeaponOut)
		{
			IsSwitchingWeaponOut = false;
		}
	}
}

void APlayerCharacter_CPP::EndSwitchWeaponSequence()
{
	if (WeaponSwitchIndex < PlayerWeapons.Num() && PlayerWeapons[WeaponSwitchIndex] != nullptr)
	{
		//SelectedWeapon->HideWeaponMesh();
		SelectedWeapon->SetActorHiddenInGame(true);

		SelectedWeapon = PlayerWeapons[WeaponSwitchIndex];
		AnimationInstance->SetSelectedWeapon(PlayerWeapons[WeaponSwitchIndex]->GetWeaponType());
		UpdateAmmoUI(SelectedWeapon->GetCurrentAmmo(), SelectedWeapon->GetTotalAmmo());

		ABaseGun* Gun = Cast<ABaseGun>(SelectedWeapon);
		if (Gun)
		{
			SelectedGun = Gun;
		}
		else
		{
			SelectedGun = nullptr;
		}

		//SelectedWeapon->ShowWeaponMesh();
		SelectedWeapon->SetActorHiddenInGame(false);
	}
}

void APlayerCharacter_CPP::HandlePhysicalRecoil(float DeltaTime)
{
	/**
	* If CurrentRecoilTime is greater than 0, recoil should be simulated by adding value to player's camera input
	* and by slightly raising the player's mesh
	*/
	if (CurrentRecoilTime > 0)
	{
		if (SelectedWeapon)
		{
			// Used so that excessive recoil is not added in the event that a temporary game freeze occurs
			float DeltaTimeCorrection = (CurrentRecoilTime > DeltaTime) ? CurrentRecoilTime : DeltaTime;

			// Simulate camera recoil
			FVector2D VerticalRecoilRange = SelectedWeapon->GetWeaponVerticalRecoilPerSecond();
			FVector2D HorizontalRecoilRange = SelectedWeapon->GetWeaponHorizontalRecoilPerSecond();

			float VerticalRecoil = FMath::FRandRange(VerticalRecoilRange.X, VerticalRecoilRange.Y) * DeltaTimeCorrection;
			float HorizontalRecoil = FMath::FRandRange(HorizontalRecoilRange.X, HorizontalRecoilRange.Y) * DeltaTimeCorrection;

			CurrentRecoilVertical += VerticalRecoil;
			CurrentRecoilHorizontal += HorizontalRecoil;

			AddControllerPitchInput(-VerticalRecoil);
			AddControllerYawInput(HorizontalRecoil);

			// Simulate physical recoil
			float MeshRaiseDistance = SelectedWeapon->GetWeaponRecoilMoveDistance();

			/**
			 * Use smoothstep function to calculate a lerp alpha value so that the raising
			 * of the player's weapon is faster right after firing (is more appealing visually)
			 */
			float LerpAlpha = FMath::SmoothStep(0.0f, CurrentRecoilTime, DeltaTimeCorrection);
			CurrentMeshRaiseDistance = FMath::Lerp(0.0f, MeshRaiseDistance, LerpAlpha);

			GetMesh()->SetRelativeLocation(MeshOriginalRelativeLocation + CurrentMeshAimDistance + FVector(0, 0, CurrentMeshRaiseDistance), false);

			CurrentRecoilTime -= DeltaTime;
		}
	}

	// Recoil recovery logic
	else if (CurrentRecoilVertical > 0 || CurrentRecoilHorizontal > 0)
	{
		// Prevent current recoil time from being a big negative number if delta time value was big
		CurrentRecoilTime = 0;

		if (SelectedWeapon)
		{
			// Camera recoil recovery
			float RecoverInterpSpeed = SelectedWeapon->GetWeaponRecoilRecoverSpeed();

			float PrevRecoilVertical = CurrentRecoilVertical;
			float PrevRecoilHorizontal = CurrentRecoilHorizontal;

			CurrentRecoilVertical = FMath::FInterpTo(CurrentRecoilVertical, 0, DeltaTime, RecoverInterpSpeed);
			CurrentRecoilHorizontal = FMath::FInterpTo(CurrentRecoilHorizontal, 0, DeltaTime, RecoverInterpSpeed);

			float RecoilRecoveryVertical = PrevRecoilVertical - CurrentRecoilVertical;
			float RecoilRecoveryHorizontal = PrevRecoilHorizontal - CurrentRecoilHorizontal;

			// Add input in the opposite direction as the recoil to simulate recoil recovery
			AddControllerPitchInput(RecoilRecoveryVertical);
			AddControllerYawInput(-RecoilRecoveryHorizontal);

			// Physical recoil recovery
			float PrevMeshRaiseDistance = CurrentMeshRaiseDistance;

			CurrentMeshRaiseDistance = FMath::FInterpTo(CurrentMeshRaiseDistance, 0, DeltaTime, RecoverInterpSpeed);

			float RecoilDistanceRecovery = PrevMeshRaiseDistance - CurrentMeshRaiseDistance;

			GetMesh()->AddRelativeLocation(FVector(0, 0, -RecoilDistanceRecovery));
		}
	}
}

void APlayerCharacter_CPP::HandleWeaponSwitchOut(float DeltaTime)
{
	if (CurrentWeaponSwitchOutTime > 0)
	{
		// Weapon switch out logic
		float LerpAlpha = FMath::SmoothStep(0.0f, CurrentWeaponSwitchOutTime, DeltaTime);
		CurrentWeaponSwitchMoveDistance = FMath::Lerp(CurrentWeaponSwitchMoveDistance, WeaponSwitchMeshMoveDistance, LerpAlpha);

		GetMesh()->SetRelativeLocation(MeshOriginalRelativeLocation + CurrentWeaponSwitchMoveDistance, false);

		CurrentWeaponSwitchOutTime -= DeltaTime;

		// If true, execute post weapon switch out logic
		if (CurrentWeaponSwitchOutTime <= 0)
		{
			IsSwitchingWeaponOut = false;
			IsSwitchingWeaponIn = true;

			CurrentWeaponSwitchOutTime = 0;

			EndSwitchWeaponSequence();
		}
	}
}

void APlayerCharacter_CPP::HandleWeaponSwitchIn(float DeltaTime)
{
	if (IsSwitchingWeaponIn)
	{
		CurrentWeaponSwitchMoveDistance = FMath::VInterpTo(CurrentWeaponSwitchMoveDistance, FVector::ZeroVector, DeltaTime, WeaponSwitchInInterpSpeed);
		GetMesh()->SetRelativeLocation(MeshOriginalRelativeLocation + CurrentWeaponSwitchMoveDistance, false);

		if (UKismetMathLibrary::Vector_IsNearlyZero(CurrentWeaponSwitchMoveDistance))
		{
			IsSwitchingWeaponIn = false;
		}
	}
}

void APlayerCharacter_CPP::HandleWeaponReload(float DeltaTime)
{
	// If reloading, lerp mesh to a distance offset
	if (InitialReloadTime > 0)
	{
		// Variable IsReloading used to simplify other statements 
		IsReloading = true;

		FVector PreviousReloadMoveDistance = CurrentReloadMoveDistance;

		// Move mesh by a distance specified by WeaponReloadMeshMoveDistance
		float LerpAlpha = FMath::SmoothStep(0.0f, InitialReloadTime, DeltaTime);
		FVector DifferenceVec = UKismetMathLibrary::VLerp(CurrentReloadMoveDistance, WeaponReloadMeshMoveDistance, LerpAlpha) - PreviousReloadMoveDistance;

		GetMesh()->AddRelativeLocation(DifferenceVec);
		/**
		 * MeshRaiseDistance is the distance the mesh is raised due to recoil simulation. Use it to simulate relative location
		 */
		//GetMesh()->SetRelativeLocation(MeshOriginalRelativeLocation + CurrentReloadMoveDistance + FVector(0, 0, CurrentMeshRaiseDistance + CurrentMeshAimDistance), false);

		InitialReloadTime -= DeltaTime;
		TotalReloadTime -= DeltaTime;

		if (InitialReloadTime <= 0)
		{
			/**
			 * Setting mesh to be not visible causes a weird bug where setting the mesh back to 
			 * visible does not make the mesh visible unless camera is moved. 
			 */ 
			//GetMesh()->SetVisibility(false);
			SelectedWeapon->HideWeaponMesh();

			USoundCue* ReloadSound;
			if (!IsIntermediateReload)
			{
				ReloadSound = SelectedWeapon->GetReloadSound();
			}
			else 
			{
				ReloadSound = SelectedWeapon->GetIntermediateReloadSound();
			}
			if (ReloadSound)
			{
				UGameplayStatics::PlaySound2D(this, ReloadSound);
			}
		}
	}
	// If not reloading, interp mesh back to original location
	else if(IsFinishingReloadInterp)
	{
		FVector NewMeshLocation = UKismetMathLibrary::VInterpTo(GetMesh()->GetRelativeLocation(), MeshOriginalRelativeLocation, DeltaTime, WeaponReloadOutInterpSpeed);

		if (NewMeshLocation.Equals(GetMesh()->GetRelativeLocation(), MESH_POST_RELOAD_IS_RELOADING_TOLERANCE))
		{
			IsReloading = false;
		}

		// If the player's mesh is now sufficiently close to its original position, stop interping mesh to original position
		if (NewMeshLocation.Equals(GetMesh()->GetRelativeLocation(), MESH_POST_RELOAD_INTERP_TOLERANCE))
		{
			IsFinishingReloadInterp = false;
		}

		GetMesh()->SetRelativeLocation(NewMeshLocation);
	}
}

void APlayerCharacter_CPP::HandleHeadTilt(float DeltaTime)
{
	FRotator NewControlRotation = GetControlRotation();
	// Rotation of gun mesh in opposite direction of control rotation
	FRotator GunMeshCounterRotation;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::SanitizeFloat(NewControlRotation.Roll));
	if (ShouldTilt)
	{
		NewControlRotation.Roll = FMath::FInterpTo(NewControlRotation.Roll, DesiredHeadTilt, DeltaTime, CurrentHeadTiltInterpSpeed);
	}
	else if(ShouldTiltBack)
	{
		if (NewControlRotation.Roll > .05)
		{			
			NewControlRotation.Roll = FMath::FInterpTo(NewControlRotation.Roll, 0.0f, DeltaTime, DefaultHeadTiltBackInterpSpeed);
		}
		else
		{
			ShouldTiltBack = false;

			NewControlRotation.Roll = 0.0f;
		}
	}

	FRotator NewCurrentMeshRotation = GetMesh()->GetRelativeRotation();
	NewCurrentMeshRotation.Roll = -(NewControlRotation.Roll / 2);

	GetMesh()->SetRelativeRotation(NewCurrentMeshRotation);
	GetController()->SetControlRotation(NewControlRotation);
}

void APlayerCharacter_CPP::HandleForwardBackwardDodge(float ForwardInput)
{
	if(ShouldDoubleTapDodge)
	{
		if (ForwardInput > 0)
		{
			if (ForwardDodgeSequence == 0)
			{
				++ForwardDodgeSequence;
				ForwardDodgeInputTimer = DodgeInputTimeout;
			}
			else if (ForwardDodgeSequence == 2)
			{
				InitiateDodgeEvent();
			}

			BackwardDodgeSequence = 0;
		}
		else if (ForwardInput == 0)
		{
			// If the forward key was pressed more recently than backward key
			if (ForwardDodgeSequence == 1 && ForwardDodgeInputTimer > 0)
			{
				++ForwardDodgeSequence;
			}
			// If the backward key was pressed more recently than forward key
			else if (BackwardDodgeSequence == 1 && BackwardDodgeInputTimer > 0)
			{
				++BackwardDodgeSequence;
			}
		}
		else if (ForwardInput < 0)
		{
			if (BackwardDodgeSequence == 0)
			{
				++BackwardDodgeSequence;
				BackwardDodgeInputTimer = DodgeInputTimeout;
			}
			else if (BackwardDodgeSequence == 2)
			{
				InitiateDodgeEvent();
			}

			ForwardDodgeSequence = 0;
		}
	}
}

void APlayerCharacter_CPP::HandleRightwardLeftwardDodge(float RightwardInput)
{
	if (ShouldDoubleTapDodge)
	{
		if (RightwardInput > 0)
		{
			if (RightwardDodgeSequence == 0)
			{
				++RightwardDodgeSequence;
				RightwardDodgeInputTimer = DodgeInputTimeout;
			}
			else if (RightwardDodgeSequence == 2)
			{
				InitiateDodgeEvent();
			}

			LeftwardDodgeSequence = 0;
		}
		else if (RightwardInput == 0)
		{
			// If the forward key was pressed more recently than backward key
			if (RightwardDodgeSequence == 1 && RightwardDodgeInputTimer > 0)
			{
				++RightwardDodgeSequence;
			}
			// If the backward key was pressed more recently than forward key
			else if (LeftwardDodgeSequence == 1 && LeftwardDodgeInputTimer > 0)
			{
				++LeftwardDodgeSequence;
			}
		}
		else if (RightwardInput < 0)
		{
			if (LeftwardDodgeSequence == 0)
			{
				++LeftwardDodgeSequence;
				LeftwardDodgeInputTimer = DodgeInputTimeout;
			}
			else if (LeftwardDodgeSequence == 2)
			{
				InitiateDodgeEvent();
			}

			RightwardDodgeSequence = 0;
		}
	}
}

void APlayerCharacter_CPP::HandleDodgeInputTimers(float DeltaTime)
{
	// Consider putting dodge input timers into array and looping through array to check timers
	if (ForwardDodgeInputTimer > 0)
	{
		ForwardDodgeInputTimer -= DeltaTime;
		if(ForwardDodgeInputTimer < 0 ) { ForwardDodgeSequence = 0; }
	}
	if (BackwardDodgeInputTimer > 0)
	{
		BackwardDodgeInputTimer -= DeltaTime;
		if (BackwardDodgeInputTimer < 0) { BackwardDodgeSequence = 0; }
	}
	if (RightwardDodgeInputTimer > 0)
	{
		RightwardDodgeInputTimer -= DeltaTime;
		if (RightwardDodgeInputTimer < 0) { RightwardDodgeSequence = 0; }
	}
	if (LeftwardDodgeInputTimer > 0)
	{
		LeftwardDodgeInputTimer -= DeltaTime;
		if (LeftwardDodgeInputTimer < 0) { LeftwardDodgeSequence = 0; }
	}
}

