// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UCharacterBase.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "Player/UPlayerCharacter.h"

#include "Engine/World.h"

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

void AUCharacterBase::RegenHealth()
{
	CurrentHealth = FMath::Clamp(CurrentHealth + 0.5f, 0, MaxHealth);

	UE_LOG(LogTemp, Warning, TEXT("Current Health Healed: %f"), CurrentHealth);

	if (CurrentHealth >= MaxHealth) 
	{
		if (HealTimer.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(HealTimer);
		}
	}
}

void AUCharacterBase::DoMeleeAttack()
{
    FVector SphereLocation = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
    float SphereRadius = 100.0f;

    TArray<FOverlapResult> OverlappingActors;
    FCollisionQueryParams CollisionParams;
    CollisionParams.bReturnPhysicalMaterial = false;

	if (GetWorld()->OverlapMultiByObjectType(
		OverlappingActors,
		SphereLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams::AllObjects,
		FCollisionShape::MakeSphere(SphereRadius),
		CollisionParams
	)) {
		for (const FOverlapResult& result : OverlappingActors)
		{
			AUPlayerCharacter* Player = Cast<AUPlayerCharacter>(result.GetActor());
			if (Player) {
				Player->TakeDamage(50.0f);
			}
		}
	}

    
}

void AUCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


