// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDead);

UCLASS( ClassGroup=(Custom) )
class UUHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUHealthComponent();

	FOnDead OnDead;

	void TakeDamage(float Damage);
	void Heal(float AmountToHeal);

protected:
	//virtual void BeginPlay() override;

private:	
	void InitializeHealthComponent();
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float CurrentHealth;
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float MaxHealth;

};
