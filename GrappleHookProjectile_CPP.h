// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseProjectile.h"
#include "GrappleHookProjectile_CPP.generated.h"

/**
 * 
 */
UCLASS()
class DREAMREALM_API AGrappleHookProjectile_CPP : public ABaseProjectile
{
	GENERATED_BODY()

protected:
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void BeginPlay() override;

public:
	AGrappleHookProjectile_CPP();

	void AddForce(FVector Force);

	UPROPERTY(Category = "Setup", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* CableAttachmentPoint;
};
