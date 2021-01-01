// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "WeaponRecoilCameraShake.h"

UWeaponRecoilCameraShake::UWeaponRecoilCameraShake()
{
	OscillationDuration = .2f;
	OscillationBlendInTime = .05f;
	OscillationBlendOutTime = .05f;

	RotOscillation.Pitch.Amplitude = FMath::RandRange(1.f, 1.5f);
	RotOscillation.Pitch.Frequency = FMath::RandRange(15, 25);

	RotOscillation.Yaw.Amplitude = FMath::RandRange(1.f, 1.5f);
	RotOscillation.Yaw.Frequency = FMath::RandRange(15, 25);

	FOVOscillation.Amplitude = FMath::RandRange(1.5f, 5.0f);
	FOVOscillation.Frequency = FMath::RandRange(15, 25);
}
