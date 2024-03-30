// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UCharacterBase.h"

// Sets default values
AUCharacterBase::AUCharacterBase()
{

	PrimaryActorTick.bCanEverTick = true;

}

void AUCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AUCharacterBase::TakeDamage(float Damage)
{
	CurrentHealth -= Damage;

	UE_LOG(LogTemp, Warning, TEXT("Took %f Damage, %f Health Remaining"), Damage, CurrentHealth);

	if (CurrentHealth <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner Has Died!"));
		StartDeath();
	}
}

void AUCharacterBase::StartDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("Actor Has Died!"));
	FTimerHandle DeconstructDeathTimeHandle;
	GetWorld()->GetTimerManager().SetTimer(DeconstructDeathTimeHandle, this, &AUCharacterBase::Deconstruct, 1.0f, false, 1.0f);
}

void AUCharacterBase::SetHealth()
{
	CurrentHealth = MaxHealth;
}

void AUCharacterBase::Deconstruct()
{
	Destroy();
}

void AUCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

