// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UCharacterBase.h"
#include "UPlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
/**
 * 
 */
UCLASS()
class AUPlayerCharacter : public AUCharacterBase
{
	GENERATED_BODY()
	
public:
	//this is the construtor
	AUPlayerCharacter();

private:
	UPROPERTY(visibleAnywhere, Category = "View")
		UCameraComponent* ViewCamera;

	/*****************************************************/
	/*                       Input                       */
	/*****************************************************/
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputMappingContext* InputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* LookInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* JumpInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* ReloadInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* ShootInputAction;

	
	UFUNCTION()
		void Move(const FInputActionValue& InputValue);

	UFUNCTION()
		void Look(const FInputActionValue& InputValue);
		

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
		void Shoot();

	UFUNCTION()
		void Reload();

};
