// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Framework/UHealthComponent.h"
#include "UCharacterBase.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDead);

UCLASS()
class AUCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AUCharacterBase();

	FOnDead OnDead;

	virtual void TakeDamage(float Damage);

	virtual void DoMeleeAttack();

	virtual void SetHealth(float MaxHealth);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	bool GetIsDead() { return CurrentHealth <= 0; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	virtual void StartDeath();

	void RegenHealth();

	float HealRate = 1.0f/ 60.0f;
	FTimerHandle HealTimer;

private:
	void Deconstruct();
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float CurrentHealth;
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float MaxHealth;
		
	UPROPERTY(VisibleDefaultsOnly, Category = "AI")
		class UAIPerceptionStimuliSourceComponent* AIPerceptionSourceComp;

};
