// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/UAIController.h"

#include "BehaviorTree/BlackboardComponent.h"

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

	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AUAIController::TargetPerceptionUpdated);

	if (BehaviorTree) 
	{
		RunBehaviorTree(BehaviorTree);
	}	
}

void AUAIController::TargetPerceptionUpdated(AActor* Target, FAIStimulus Stimulus)
{
	if (GetBlackboardComponent())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			if (!GetBlackboardComponent()->GetValueAsObject(TargetBBKeyName))
			{
				GetBlackboardComponent()->SetValueAsObject(TargetBBKeyName, Target);
			}
		}
	}
}
