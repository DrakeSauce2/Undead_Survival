#include "Player/UPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "Engine/World.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/BlueprintCore.h"

#include "Kismet/KismetSystemLibrary.h"

#include "GameFramework/CharacterMovementComponent.h"

AUPlayerCharacter::AUPlayerCharacter()
{
	ViewCamera = CreateDefaultSubobject<UCameraComponent>("View Camera");
	ViewCamera->SetupAttachment(GetRootComponent());

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	WeaponMesh->SetupAttachment(ViewCamera);

	RecoilTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RecoilTimeline"));
	RecoilAnimationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RecoilAnimationTimeline"));

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(1080.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
}

void AUPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	FOnTimelineFloat RecoilTimelineCallback;
	RecoilTimelineCallback.BindUFunction(this, FName("UpdateRecoil"));
	RecoilTimeline->AddInterpFloat(RecoilCurve, RecoilTimelineCallback);

	FOnTimelineFloat RecoilAnimTimelineCallback;
	RecoilAnimTimelineCallback.BindUFunction(this, FName("UpdateRecoilAnimation"));
	RecoilAnimationTimeline->AddInterpFloat(RecoilAnimCurve, RecoilAnimTimelineCallback);

	GetWeaponStats();
}

void AUPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	APlayerController* PlayerController = GetController<APlayerController>();
	if (PlayerController) {
		UE_LOG(LogTemp, Warning, TEXT("Player Character Has Player Controller!"));
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		InputSubsystem->ClearAllMappings();
		InputSubsystem->AddMappingContext(inputMapping, 0);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No Player Controller On Player Character!"));
	}
	
	UEnhancedInputComponent* enhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (enhancedInputComp)
	{
		enhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::Move);
		enhancedInputComp->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::Look);
		enhancedInputComp->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::Jump);
		enhancedInputComp->BindAction(ShootInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::Shoot);
		enhancedInputComp->BindAction(ReloadInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::Reload);
	}
	
}

void AUPlayerCharacter::Move(const FInputActionValue& InputValue)
{
	FVector2D input = InputValue.Get<FVector2D>();
	input.Normalize();

	AddMovementInput(input.Y * ViewCamera->GetForwardVector() + input.X * ViewCamera->GetRightVector());
}

void AUPlayerCharacter::Look(const FInputActionValue& InputValue)
{
	FVector2D input = InputValue.Get<FVector2D>();
	AddControllerYawInput(input.Y);
	AddControllerPitchInput(-input.X);
}



void AUPlayerCharacter::Shoot()
{
	if (Cur_WeaponData) 
	{
		if (Cur_WeaponData->FiringType == FireType::SingleFire) 
		{
			FVector StartLocation = ViewCamera->GetComponentLocation();
			FVector EndLocation = ViewCamera->GetForwardVector() * Cur_WeaponData->FireMaxRange + ViewCamera->GetComponentLocation();
			ECollisionChannel TraceChannel = ECC_Visibility;
			FCollisionQueryParams CollisionParams;
			CollisionParams.bTraceComplex = true; 
			CollisionParams.AddIgnoredActor(this);

			FHitResult HitResult;
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult,
				StartLocation,
				EndLocation,
				TraceChannel,
				CollisionParams
			);
			if (bHit)
			{
				AActor* HitActor = HitResult.GetActor();
				if (HitActor)
				{
					UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitActor->GetName());
				}
			}

			if (RecoilTimeline && RecoilCurve)
			{
				RecoilTimeline->SetPlayRate(1 / Cur_WeaponData->RecoilLength);

				RecoilTimeline->PlayFromStart();
				RecoilAnimationTimeline->PlayFromStart();
			}
			

			return;
		}

		if (Cur_WeaponData->FiringType == FireType::Automatic)
		{
			FVector StartLocation = ViewCamera->GetComponentLocation();
			FVector EndLocation = ViewCamera->GetForwardVector() * Cur_WeaponData->FireMaxRange + ViewCamera->GetComponentLocation();
			ECollisionChannel TraceChannel = ECC_Visibility;
			FCollisionQueryParams CollisionParams;
			CollisionParams.bTraceComplex = true;
			CollisionParams.AddIgnoredActor(this);

			FHitResult HitResult;
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult,
				StartLocation,
				EndLocation,
				TraceChannel,
				CollisionParams
			);
			if (bHit)
			{
				AActor* HitActor = HitResult.GetActor();
				if (HitActor)
				{
					UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitActor->GetName());
				}
			}

			
			if (RecoilTimeline && RecoilCurve)
			{
				RecoilTimeline->SetPlayRate(1 / Cur_WeaponData->RecoilLength);

				RecoilTimeline->PlayFromStart();
				RecoilAnimationTimeline->PlayFromStart();
			}
			

			return;
		}

	}
}

void AUPlayerCharacter::UpdateRecoil(float Value)
{
	APlayerController* PlayerController = GetController<APlayerController>();

	float TargetPitch;
	if (PlayerController->GetControlRotation().Pitch > 180) {
		TargetPitch = PlayerController->GetControlRotation().Pitch - 360;
	}
	else {
		TargetPitch = PlayerController->GetControlRotation().Pitch;
	}

	FRotator TargetRotation = FRotator
	(
		FMath::Lerp(TargetPitch, TargetPitch + Cur_WeaponData->RecoilAmount, Value),
		PlayerController->GetControlRotation().Yaw,
		PlayerController->GetControlRotation().Roll
	);

	PlayerController->SetControlRotation(TargetRotation);
	AddControllerYawInput(FMath::RandRange(-Cur_WeaponData->RecoilYaw, Cur_WeaponData->RecoilYaw));

}

void AUPlayerCharacter::UpdateRecoilAnimation(float Value)
{
	SetRecoilVariables();

	RecoilAnimation(RecoilAnimCurve->GetFloatValue(Value));

	if (RecoilAnimationTimeline->GetPlaybackPosition() >= RecoilAnimationTimeline->GetTimelineLength()) {
		bVariablesSet = false;
		UE_LOG(LogTemp, Warning, TEXT("Recoil Animation Finished!"));
	}
}

void AUPlayerCharacter::TimelineAnimFinished()
{
	bVariablesSet = true;
}


void AUPlayerCharacter::Reload()
{
	UE_LOG(LogTemp, Warning, TEXT("Reloading!"));

}

void AUPlayerCharacter::GetWeaponStats()
{
	if (WeaponDataTable) 
	{
		FName rowName = FName(*FString::FromInt(WeaponIndex));
		Cur_WeaponData = WeaponDataTable->FindRow<FUWeaponData>(rowName, TEXT(""));
		if (Cur_WeaponData)
		{
			WeaponMesh->SetSkeletalMeshAsset(Cur_WeaponData->TargetWeaponMesh);
			WeaponMesh->SetRelativeTransform(Cur_WeaponData->Transform);
		}
		
	}
}

void AUPlayerCharacter::SetRecoilVariables()
{
	if (bVariablesSet == true) return;

	if (WeaponDataTable) {
		PreRecoilRotation = WeaponMesh->GetRelativeRotation().Roll;
		RecoilRotAmount = WeaponMesh->GetRelativeRotation().Roll + -Cur_WeaponData->RecoilAmount;

		PreRecoilLocation = WeaponMesh->GetRelativeLocation().X;
		RecoilLocationAmount = WeaponMesh->GetRelativeLocation().X + -Cur_WeaponData->PullBackAmount;

		bVariablesSet = true;
	}
}

void AUPlayerCharacter::RecoilAnimation(float Alpha)
{
	FRotator TargetRotation = FRotator
	(
		WeaponMesh->GetRelativeRotation().Pitch,
		WeaponMesh->GetRelativeRotation().Yaw,
		FMath::Lerp(PreRecoilRotation, RecoilRotAmount, RecoilAnimCurve->GetFloatValue(Alpha))
	);
	WeaponMesh->SetRelativeRotation(TargetRotation);

	FVector TargetLocation = FVector
	(
		FMath::Lerp(PreRecoilLocation, RecoilLocationAmount, RecoilAnimCurve->GetFloatValue(Alpha)),
		WeaponMesh->GetRelativeLocation().Y,
		WeaponMesh->GetRelativeLocation().Z
	);
	WeaponMesh->SetRelativeLocation(TargetLocation);
}

