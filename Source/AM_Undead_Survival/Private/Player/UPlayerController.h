// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AUPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void OnPossess(APawn* NewPawn) override;
	virtual void BeginPlay() override;

private:
	UPROPERTY()
		class AUPlayerCharacter* PlayerCharacter;


};
