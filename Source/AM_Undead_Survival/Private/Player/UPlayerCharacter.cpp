#include "Player/UPlayerCharacter.h"

#include "Camera/CameraComponent.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

AUPlayerCharacter::AUPlayerCharacter()
{
	ViewCamera = CreateDefaultSubobject<UCameraComponent>("View Camera");

	ViewCamera->SetupAttachment(GetRootComponent());

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(1080.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
}

void AUPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* enhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (enhancedInputComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Has Comp!"));

		enhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::Move);
		enhancedInputComp->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::Look);
		enhancedInputComp->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::Jump);
		enhancedInputComp->BindAction(ShootInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::Shoot);
		enhancedInputComp->BindAction(ReloadInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::Reload);
	}
	
}


void AUPlayerCharacter::Move(const FInputActionValue& InputValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Moving!"));

	FVector2D input = InputValue.Get<FVector2D>();
	input.Normalize();

	AddMovementInput(input.Y * GetActorForwardVector() + input.X * GetActorRightVector());
}

void AUPlayerCharacter::Look(const FInputActionValue& InputValue)
{
	FVector2D input = InputValue.Get<FVector2D>();
	//AddControllerYawInput(input.X);
	//AddControllerPitchInput(-input.Y);
}


void AUPlayerCharacter::Shoot()
{
	UE_LOG(LogTemp, Warning, TEXT("Shooting!"));

}

void AUPlayerCharacter::Reload()
{
	UE_LOG(LogTemp, Warning, TEXT("Reloading!"));

}
