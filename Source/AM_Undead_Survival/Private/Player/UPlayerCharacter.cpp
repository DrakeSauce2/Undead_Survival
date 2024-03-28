#include "Player/UPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"

#include "DrawDebugHelpers.h"

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
	AimAnimationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AimAnimationTimeline"));

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(1080.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
}

void AUPlayerCharacter::UpdateAmmo(int32 NewAmmo, int32 NewTotalAmmo)
{
	OnAmmoChanged.Broadcast(NewAmmo, NewTotalAmmo);
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

	RecoilAnimationTimeline->SetTimelineFinishedFunc(FOnTimelineEventStatic::CreateUFunction(this, FName("RecoilTimelineAnimFinished")));

	FOnTimelineFloat AimAnimTimelineCallback;
	AimAnimTimelineCallback.BindUFunction(this, FName("UpdateAimAnimation"));
	AimAnimationTimeline->AddInterpFloat(AimAnimCurve, AimAnimTimelineCallback);

	SetWeaponStats();

	WeaponStartingLocation = WeaponMesh->GetRelativeLocation();
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

		enhancedInputComp->BindAction(AimInputAction, ETriggerEvent::Started, this, &AUPlayerCharacter::Aim);
		enhancedInputComp->BindAction(AimInputAction, ETriggerEvent::Completed, this, &AUPlayerCharacter::UnAim);

		enhancedInputComp->BindAction(ShootInputAction, ETriggerEvent::Started, this, &AUPlayerCharacter::Shoot);
		enhancedInputComp->BindAction(ShootInputAction, ETriggerEvent::Completed, this, &AUPlayerCharacter::StoppedShooting);

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
	if (!Cur_WeaponData) 
	{
		return;
	}

	if (AmmoClipCur > 0)
	{
		if (Cur_WeaponData->FiringType == FireType::Automatic)
		{
			if (!AutoFireTimerHandle.IsValid())
			{
				GetWorld()->GetTimerManager().SetTimer(AutoFireTimerHandle, this, &AUPlayerCharacter::AutoFire, Cur_WeaponData->FireRate, true);
			}

			return;
		}

		if (Cur_WeaponData->FiringType == FireType::SingleFire)
		{
			if (bWeaponDelay == false) {
				AActor* HitActor = FiringLineTrace();

				bWeaponDelay = true;
			}

			if (!WeaponDelayTimerHandle.IsValid())
			{
				GetWorld()->GetTimerManager().SetTimer(WeaponDelayTimerHandle, this, &AUPlayerCharacter::ToggleWeaponDelay, WeaponDelayRate, false, -Cur_WeaponData->FireRate);
			}

			return;
		}
	}
}

void AUPlayerCharacter::DecrementClipAmmo()
{
	if (Cur_WeaponData) {
		if (AmmoClipCur > 0) 
		{
			AmmoClipCur = FMath::Clamp(AmmoClipCur - 1, 0, Cur_WeaponData->AmmoClipMax);
			UpdateAmmo(AmmoClipCur, AmmoTotalCur);
		}
		else 
		{
			StoppedShooting();
		}
	}
}

void AUPlayerCharacter::StoppedShooting()
{
	if (Cur_WeaponData->FiringType != FireType::Automatic)
	{
		return;
	}

	if (AutoFireTimerHandle.IsValid()) {
		UE_LOG(LogTemp, Warning, TEXT("AutoFireTimerHandle Cleared!"));
		GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
	}
}

void AUPlayerCharacter::Aim()
{
	bIsAiming = true;
	AimAnimationTimeline->Play();
}

void AUPlayerCharacter::UnAim()
{
	bIsAiming = false;
	AimAnimationTimeline->Reverse();
}



void AUPlayerCharacter::UpdateAimAnimation(float Value)
{
	FVector AimLocation = FVector
	(
		-WeaponMesh->GetSocketTransform(FName("Aim_Socket"), ERelativeTransformSpace::RTS_ParentBoneSpace).GetLocation().Z,
		0,
		WeaponMesh->GetSocketTransform(FName("Aim_Socket"), ERelativeTransformSpace::RTS_ParentBoneSpace).GetLocation().Y
	);
	FVector TargetLocation = FMath::Lerp(WeaponStartingLocation, AimLocation, Value);
	WeaponMesh->SetRelativeLocation(TargetLocation);

	ViewCamera->SetFieldOfView(FMath::Lerp(90, Cur_WeaponData->AimFOV, Value));
}

void AUPlayerCharacter::SetWeaponStats()
{
	if (WeaponDataTable)
	{
		FName rowName = FName(*FString::FromInt(WeaponIndex));
		Cur_WeaponData = WeaponDataTable->FindRow<FUWeaponData>(rowName, TEXT(""));
		if (Cur_WeaponData)
		{
			WeaponMesh->SetSkeletalMeshAsset(Cur_WeaponData->TargetWeaponMesh);
			WeaponMesh->SetRelativeTransform(Cur_WeaponData->Transform);

			AmmoClipCur = Cur_WeaponData->AmmoClipMax;
			AmmoTotalCur = Cur_WeaponData->AmmoTotalMax - Cur_WeaponData->AmmoClipMax;
		}
	}
}

void AUPlayerCharacter::AutoFire()
{
	AActor* HitActor = FiringLineTrace(); // Have to line trace here due to input type

	Recoil();

}

void AUPlayerCharacter::ToggleWeaponDelay()
{
	bWeaponDelay = false;
	GetWorld()->GetTimerManager().ClearTimer(WeaponDelayTimerHandle);

}

void AUPlayerCharacter::Reload()
{
	UE_LOG(LogTemp, Warning, TEXT("Reloading!"));

}

AActor* AUPlayerCharacter::FiringLineTrace()
{
	float BulletSpread = 0.0f;
	if (!bIsAiming) {
		BulletSpread = -Cur_WeaponData->BulletSpread;
	}

	FVector StartLocation = ViewCamera->GetComponentLocation();
	FVector ForwardEndVector = ViewCamera->GetForwardVector() * Cur_WeaponData->FireMaxRange;
	FVector RightEndVector = ViewCamera->GetRightVector() * FMath::RandRange(-BulletSpread, BulletSpread);
	FVector UpEndVector = ViewCamera->GetUpVector() * FMath::RandRange(-BulletSpread, BulletSpread);
	FVector TargetEndVector = ForwardEndVector + RightEndVector + UpEndVector;

	FVector EndLocation = TargetEndVector;
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
			//UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitActor->GetName());
			DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Red, false, 3.0f, 0);

			DecrementClipAmmo();

			Recoil();

			return HitActor;
		}
	}

	DecrementClipAmmo();

	Recoil();

	return nullptr;
}

#pragma region Recoil Functions

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

	RecoilAnimation(Value);

}

void AUPlayerCharacter::RecoilTimelineAnimFinished()
{
	bVariablesSet = false;
	UE_LOG(LogTemp, Warning, TEXT("Recoil Animation Finished!"));
}

void AUPlayerCharacter::Recoil()
{
	if (RecoilTimeline && RecoilCurve)
	{
		RecoilTimeline->SetPlayRate(1 / Cur_WeaponData->RecoilLength);

		RecoilTimeline->PlayFromStart();
		RecoilAnimationTimeline->PlayFromStart();
	}
}

void AUPlayerCharacter::SetRecoilVariables()
{
	if (bVariablesSet == true) 
	{
		return;
	}

	PreRecoilRotation = WeaponMesh->GetRelativeRotation().Roll;
	RecoilRotAmount = WeaponMesh->GetRelativeRotation().Roll + -Cur_WeaponData->RecoilAmount;

	PreRecoilLocation = WeaponMesh->GetRelativeLocation().X;
	RecoilLocationAmount = WeaponMesh->GetRelativeLocation().X + -Cur_WeaponData->PullBackAmount;

	bVariablesSet = true;
}

void AUPlayerCharacter::RecoilAnimation(float Alpha)
{
	FRotator TargetRotation = FRotator
	(
		WeaponMesh->GetRelativeRotation().Pitch,
		WeaponMesh->GetRelativeRotation().Yaw,
		FMath::Lerp(PreRecoilRotation, RecoilRotAmount, Alpha)
	);
	WeaponMesh->SetRelativeRotation(TargetRotation);

	FVector TargetLocation = FVector
	(
		FMath::Lerp(PreRecoilLocation, RecoilLocationAmount, Alpha),
		WeaponMesh->GetRelativeLocation().Y,
		WeaponMesh->GetRelativeLocation().Z
	);
	WeaponMesh->SetRelativeLocation(TargetLocation);
}

#pragma endregion
