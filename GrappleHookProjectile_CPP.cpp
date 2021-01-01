// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "GrappleHookProjectile_CPP.h"
#include "GrappleBelt.h"

AGrappleHookProjectile_CPP::AGrappleHookProjectile_CPP()
{
	CableAttachmentPoint = CreateDefaultSubobject<USceneComponent>("CableAttachmentPoint");
	
	SetRootComponent(ProjectileMesh);
	CableAttachmentPoint->SetupAttachment(ProjectileMesh);
}

void AGrappleHookProjectile_CPP::BeginPlay()
{
	Super::BeginPlay();

	CableAttachmentPoint->AttachToComponent(ProjectileMesh, FAttachmentTransformRules::KeepRelativeTransform);

	EnableCollisionEvents();
}

void AGrappleHookProjectile_CPP::AddForce(FVector Force)
{
	ProjectileMesh->AddForce(Force);
}

void AGrappleHookProjectile_CPP::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AGrappleBelt* GrappleBelt = Cast<AGrappleBelt>(GetOwner());
	if (GrappleBelt)
	{
		GrappleBelt->OnHookLand(HitComponent, OtherActor, NormalImpulse, Hit);
	}
}


