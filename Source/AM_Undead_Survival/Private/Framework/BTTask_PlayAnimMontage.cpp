// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/BTTask_PlayAnimMontage.h"

#include "AIController.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "Character/UCharacterBase.h"

UBTTask_PlayAnimMontage::UBTTask_PlayAnimMontage()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PlayAnimMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AUCharacterBase* Character = OwnerComp.GetAIOwner()->GetPawn<AUCharacterBase>();
	if (Character)
	{
		float MontageLength = Character->PlayAnimMontage(MontageToPlay);

		//Character->PlayAnimMontage(MontageToPlay);

		PlayMontageNodeData* Data = GetSpecialNodeMemory<PlayMontageNodeData>(NodeMemory);

		if (Data)
		{
			Data->MontagePlayed = MontageToPlay;
			Data->MontageTimeLeft = MontageLength;
		}

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_PlayAnimMontage::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	PlayMontageNodeData* Data = GetSpecialNodeMemory<PlayMontageNodeData>(NodeMemory);
	if (Data)
	{
		Data->MontageTimeLeft -= DeltaSeconds;
		if (Data->MontageTimeLeft <= 0)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}

uint16 UBTTask_PlayAnimMontage::GetSpecialMemorySize() const
{
	return sizeof(PlayMontageNodeData);
}
