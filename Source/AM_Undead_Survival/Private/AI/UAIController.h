// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "UAIController.generated.h"

/**
 * 
 */
UCLASS()
class AUAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AUAIController();

	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "AI")
		class UAIPerceptionComponent* AIPerceptionComponent;
	UPROPERTY(VisibleDefaultsOnly, Category = "AI")
		class UAISenseConfig_Sight* SightConfig;
	UPROPERTY(EditDefaultsOnly, Category = "AI")
		class UBehaviorTree* BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName TargetBBKeyName = "TargetLocation";



};
