// Copyright (C) Brandon DeSiata. 2020. All Rights Reserved.


#include "GrappleCable_CPP.h"

AGrappleCable_CPP::AGrappleCable_CPP()
{

}

void AGrappleCable_CPP::BeginPlay()
{
	Super::BeginPlay();

	// Make cable not cast shadow
	CableComponent->CastShadow = 0;
}
