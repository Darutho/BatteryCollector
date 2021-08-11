// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "BatteryCollectorGameMode.h"
#include "BatteryCollectorCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "SpawnVolume.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
//#include "GameFramework/PawnMovementComponent.h"﻿

ABatteryCollectorGameMode::ABatteryCollectorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	DecayRate = 0.03f;
}

void ABatteryCollectorGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundActors);

	for (auto Actor : FoundActors)
	{
		ASpawnVolume* SpawnVolumeActor = Cast<ASpawnVolume>(Actor);
		if (SpawnVolumeActor)
		{
			SpawnVolumeActors.AddUnique(SpawnVolumeActor);
		}
	}

	SetCurrState(EBatteryPlayState::EPlaying);

	ABatteryCollectorCharacter* MyChar = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyChar)
	{
		PowerToWin = (MyChar->GetInitialPower())*1.25f;
	}

	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}
}

void ABatteryCollectorGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ABatteryCollectorCharacter* MyChar = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyChar)
	{
		if (MyChar->GetCurrentPower() > PowerToWin)
			SetCurrState(EBatteryPlayState::EWon);
		else if(MyChar->GetCurrentPower()>0)
			MyChar->UpdatePower(-DeltaTime*DecayRate*(MyChar->GetInitialPower()));
		else 
			SetCurrState(EBatteryPlayState::EGameOver);
	}
}

float ABatteryCollectorGameMode::GetPowerToWin() const
{
	return PowerToWin;
}

EBatteryPlayState ABatteryCollectorGameMode::GetCurrState() const
{
	return CurrState;
}

void ABatteryCollectorGameMode::SetCurrState(EBatteryPlayState NewState)
{
	CurrState = NewState;
	HandleNewState(CurrState);
}

void ABatteryCollectorGameMode::HandleNewState(EBatteryPlayState NewState)
{
	switch (NewState)
	{
		case EBatteryPlayState::EPlaying:
		{
			for (ASpawnVolume* Volume : SpawnVolumeActors)
			{
				Volume->SetSpawningActive(true);
			}
		}
		break;
		case EBatteryPlayState::EWon:
		{
			for (ASpawnVolume* Volume : SpawnVolumeActors)
			{
				Volume->SetSpawningActive(false);
			}
		}
		break;
		case EBatteryPlayState::EGameOver:
		{
			for (ASpawnVolume* Volume : SpawnVolumeActors)
			{
				Volume->SetSpawningActive(true);
			}

			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
			if (PlayerController)
			{
				PlayerController->SetCinematicMode(true, false, false, true, true);
				PlayerController->GetPawnOrSpectator()->DisableInput(PlayerController);
			}

			ACharacter* MyChar = UGameplayStatics::GetPlayerCharacter(this,0);
			if (MyChar)
			{
				MyChar->GetMesh()->SetSimulatePhysics(true);
				//MyChar->GetMovementComponent()->MovementState.bCanJump = false;
			}
		}
		break;
		default:
		case EBatteryPlayState::EUnknown:
		{

		}
		break;
	}
}