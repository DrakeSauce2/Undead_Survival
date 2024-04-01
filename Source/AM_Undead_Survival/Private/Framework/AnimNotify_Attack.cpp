// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AnimNotify_Attack.h"

#include "Components/SkeletalMeshComponent.h"
#include "Character/UCharacterBase.h"

#include "Kismet/GameplayStatics.h"

void UAnimNotify_Attack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp->GetOwner()) 
	{
		AUCharacterBase* OwnerCharacter = Cast<AUCharacterBase>(MeshComp->GetOwner());
		if (OwnerCharacter)
		{
			UE_LOG(LogTemp, Warning, TEXT("Attack Notified!"));
			OwnerCharacter->DoMeleeAttack();
		}
	}

}
