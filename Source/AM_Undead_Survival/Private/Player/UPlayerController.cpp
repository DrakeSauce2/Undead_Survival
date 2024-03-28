// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UPlayerController.h"
#include "Player/UPlayerCharacter.h"
#include "Widgets/GameplayUI.h"

void AUPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	PlayerCharacter = Cast<AUPlayerCharacter>(NewPawn);

	SpawnGameplayUI();
}

void AUPlayerController::BeginPlay()
{
	Super::BeginPlay();

	
}

void AUPlayerController::SpawnGameplayUI()
{
	GameplayUI = CreateWidget<UGameplayUI>(this, GameplayUIClass);
	if (GameplayUI)
	{
		GameplayUI->AddToViewport();
		GameplayUI->SetOwningPlayerCharacter(PlayerCharacter);
	}
}
