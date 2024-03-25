// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UPlayerController.h"
#include "Player/UPlayerCharacter.h"

void AUPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	PlayerCharacter = Cast<AUPlayerCharacter>(NewPawn);

}

void AUPlayerController::BeginPlay()
{
	Super::BeginPlay();
}
