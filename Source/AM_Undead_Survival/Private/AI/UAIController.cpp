// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/UAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"

#include "GameFramework/Character.h"

#include "Kismet/GameplayStatics.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AUAIController::AUAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

	SightConfig->PeripheralVisionAngleDegrees = 360.0f;
	SightConfig->SightRadius = 50000.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
}

void AUAIController::BeginPlay()
{
	Super::BeginPlay();

	if (BehaviorTree) 
	{
		RunBehaviorTree(BehaviorTree);
	}	

	AIPerceptionComponent->Activate(true);

	GetBrainComponent()->StartLogic();
}

void AUAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ACharacter* const Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		FVector const PlayerLocation = Player->GetActorLocation();

		GetBlackboardComponent()->SetValueAsVector(TargetBBKeyName, PlayerLocation);
	}

}
