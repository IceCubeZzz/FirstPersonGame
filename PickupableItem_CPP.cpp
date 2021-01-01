// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "PickupableItem_CPP.h"

// Sets default values
APickupableItem_CPP::APickupableItem_CPP()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APickupableItem_CPP::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APickupableItem_CPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int APickupableItem_CPP::Use_Implementation()
{
	return 0;
}

FString APickupableItem_CPP::UseText_Implementation()
{
	return "ye";
}

