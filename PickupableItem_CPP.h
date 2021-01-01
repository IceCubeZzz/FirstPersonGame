// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InterfaceUsable.h"
#include "PickupableItem_CPP.generated.h"

UCLASS()
class DREAMREALM_API APickupableItem_CPP : public AActor, public IInterfaceUsable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupableItem_CPP();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int Use();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FString UseText();
};
