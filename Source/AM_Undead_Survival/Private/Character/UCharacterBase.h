// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Framework/UHealthComponent.h"
#include "UCharacterBase.generated.h"

class UUHealthComponent;

UCLASS()
class AUCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AUCharacterBase();

	void TakeDamage(float Damage);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	virtual void StartDeath();

	virtual void SetHealth();

private:
	void Deconstruct();
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float CurrentHealth;
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float MaxHealth;
		
	UPROPERTY(VisibleDefaultsOnly, Category = "AI")
		class UAIPerceptionStimuliSourceComponent* AIPerceptionSourceComp;

};
