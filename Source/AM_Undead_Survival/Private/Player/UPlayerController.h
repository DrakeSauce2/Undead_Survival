// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
/**
 * 
 */
UCLASS()
class AUPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void OnPossess(APawn* NewPawn) override;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
		void SpawnGameplayUI();

	UPROPERTY()
		class AUPlayerCharacter* PlayerCharacter;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<class UGameplayUI> GameplayUIClass;
	UPROPERTY()
		UGameplayUI* GameplayUI;

};
