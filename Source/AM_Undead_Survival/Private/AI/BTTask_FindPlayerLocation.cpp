// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_FindPlayerLocation.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UBTTask_FindPlayerLocation::UBTTask_FindPlayerLocation(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Find Player Location");
}

EBTNodeResult::Type UBTTask_FindPlayerLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ACharacter* const Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)) 
	{
		FVector const PlayerLocation = Player->GetActorLocation();

		OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), PlayerLocation);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
