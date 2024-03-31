// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UUAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovemmentComp = OwnerCharacter->GetCharacterMovement();
	}
}

void UUAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter && OwnerMovemmentComp)
	{
		Speed = OwnerCharacter->GetVelocity().Length();
	}

}
