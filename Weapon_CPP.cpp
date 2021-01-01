// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "Weapon_CPP.h"

// Sets default values
AWeapon_CPP::AWeapon_CPP()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWeapon_CPP::BeginPlay()
{
	Super::BeginPlay();

	// Deactivate components that are not in use
	if (!(WeaponMeshStatic->GetStaticMesh()))
	{
		WeaponMeshStatic->Deactivate();
	}
	if (!(WeaponMeshSkeletal->SkeletalMesh))
	{
		WeaponMeshSkeletal->Deactivate();
	}
}

// Called every frame
void AWeapon_CPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon_CPP::BeginReload()
{

}

void AWeapon_CPP::SetLocationToFireTowards(FVector NewLocation)
{

}

void AWeapon_CPP::FireTowardsLocation()
{

}

bool AWeapon_CPP::Fire()
{
	return true;
}

bool AWeapon_CPP::BeginReloadAnimation()
{
	return true;
}

UAnimSequenceBase* AWeapon_CPP::GetPlayerMeshFireAnimation()
{
	return PlayerMeshFireAnimation;
}

UAnimSequenceBase* AWeapon_CPP::GetPlayerMeshReloadAnimation()
{
	return PlayerMeshReloadAnimation;
}

bool AWeapon_CPP::GetIsAutomatic()
{
	return IsAutomatic;
}

int AWeapon_CPP::GetCurrentAmmo()
{
	return -1;
}

int AWeapon_CPP::GetTotalAmmo()
{
	return -1;
}

float AWeapon_CPP::GetTimeToReload()
{
	return TimeToReload;
}

float AWeapon_CPP::GetTimeToReloadIntermediate()
{
	return TimeToReloadIntermediate;
}

bool AWeapon_CPP::GetIntermediateReloadCompleted()
{
	return IntermediateReloadCompleted;
}

void AWeapon_CPP::SetIntermediateReloadCompleted(bool Completed)
{
	IntermediateReloadCompleted = Completed;
}

USoundCue* AWeapon_CPP::GetReloadSound()
{
	return ReloadSound;
}

USoundCue* AWeapon_CPP::GetIntermediateReloadSound()
{
	return IntermediateReloadSound;
}

float AWeapon_CPP::GetWeaponDamage()
{
	return WeaponDamage;
}

float AWeapon_CPP::GetWeaponSwitchTime()
{
	return WeaponSwitchTime;
}

float AWeapon_CPP::GetWeaponRecoilCameraShakeScale()
{
	return WeaponRecoilCameraShakeScale;
}

FVector2D AWeapon_CPP::GetWeaponVerticalRecoilPerSecond()
{
	return WeaponVerticalRecoilRange;
}

FVector2D AWeapon_CPP::GetWeaponHorizontalRecoilPerSecond()
{
	return WeaponHorizontalRecoilRange;
}

float AWeapon_CPP::GetWeaponRecoilRecoverSpeed()
{
	return WeaponRecoilRecoverSpeed;
}

float AWeapon_CPP::GetWeaponRecoilIncreaseTime()
{
	return WeaponRecoilIncreaseTime;
}

float AWeapon_CPP::GetWeaponRecoilMoveDistance()
{
	return WeaponRecoilMoveDistance;
}

EWeaponType AWeapon_CPP::GetWeaponType()
{
	return Weapon;
}

void AWeapon_CPP::HideWeaponMesh()
{
	if (WeaponMeshStatic->IsActive())
	{
		WeaponMeshStatic->SetVisibility(false);
	}
	if (WeaponMeshSkeletal->IsActive())
	{
		WeaponMeshSkeletal->SetVisibility(false);
	}
}

void AWeapon_CPP::ShowWeaponMesh()
{
	if (WeaponMeshStatic->IsActive())
	{
		WeaponMeshStatic->SetVisibility(true);
	}
	if (WeaponMeshSkeletal->IsActive())
	{
		WeaponMeshSkeletal->SetVisibility(true);
	}
}

UStaticMeshComponent* AWeapon_CPP::GetWeaponStaticMeshComponent()
{
	return WeaponMeshStatic;
}

USkeletalMeshComponent* AWeapon_CPP::GetWeaponSkeletalMeshComponent()
{
	return WeaponMeshSkeletal;
}

void AWeapon_CPP::PlayGunFireAnimation()
{

}

/*void AWeapon_CPP::SetProjectileNonSocketSpawnPoint(FVector SpawnPoint)
{
	ProjectileNonSocketSpawnPoint = SpawnPoint;
}

void AWeapon_CPP::SetShouldFireFromSocket(bool FireFromSocket)
{
	ShouldFireFromSocket = FireFromSocket;
}

bool AWeapon_CPP::GetShouldFireFromSocket()
{
	return ShouldFireFromSocket;
}*/

