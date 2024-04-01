// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UCharacterBase.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

// Sets default values
AUCharacterBase::AUCharacterBase()
{

	PrimaryActorTick.bCanEverTick = true;

	AIPerceptionSourceComp = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("AI Perception Source Comp");
	AIPerceptionSourceComp->RegisterForSense(UAISense_Sight::StaticClass());
	AIPerceptionSourceComp->RegisterWithPerceptionSystem();

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
		StartDeath();
		return;
	}
}

void AUCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUCharacterBase::StartDeath()
{
	if (bIsDead == true) return;

	bIsDead = true;

	OnDead.Broadcast();

	Deconstruct();
}

void AUCharacterBase::SetHealth(float NewMaxHealth)
{
	CurrentHealth = NewMaxHealth;
}

void AUCharacterBase::Deconstruct()
{
	Destroy();
}

void AUCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


