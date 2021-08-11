// Fill out your copyright notice in the Description page of Project Settings.

#include "BatteryPickup.h"

ABatteryPickup::ABatteryPickup()
{
	GetMesh()->SetSimulatePhysics(true);

	BatteryPower = 150.f;
}

void ABatteryPickup::WasCollected_Implementation()
{
	//Use base pickup behavior
	Super::WasCollected_Implementation();
	Destroy();
}

float ABatteryPickup::GetPower()
{
	return BatteryPower;
}