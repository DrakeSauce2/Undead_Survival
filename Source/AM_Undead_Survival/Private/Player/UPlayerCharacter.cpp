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
	ReloadAnimationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ReloadAnimationTimeline"));
	ReloadEventTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ReloadEventTimeline"));

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

	InitializeTimelines();

	SetWeaponStats();

	GetWorld()->GetTimerManager().SetTimer(WeaponSwayTimerHandle, this, &AUPlayerCharacter::WeaponSway, WeaponSwayRate, true);

	WeaponStartingLocation = WeaponMesh->GetRelativeLocation();
}


void AUPlayerCharacter::InitializeTimelines()
{
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
	AimAnimationTimeline->SetTimelineFinishedFunc(FOnTimelineEventStatic::CreateUFunction(this, FName("SetRecoilVariablesAfterAiming")));


	FOnTimelineFloat ReloadAnimTimelineCallback;
	ReloadAnimTimelineCallback.BindUFunction(this, FName("UpdateReloadAnimation"));
	ReloadAnimationTimeline->AddInterpFloat(ReloadAnimCurve, ReloadAnimTimelineCallback);
	ReloadAnimationTimeline->SetTimelineFinishedFunc(FOnTimelineEventStatic::CreateUFunction(this, FName("ReloadEventTimelineFinished")));

	FOnTimelineFloat ReloadEventTimelineCallback;
	ReloadEventTimelineCallback.BindUFunction(this, FName("UpdateReloadAnimation"));
	FOnTimelineEvent ReloadActionEvent;
	ReloadActionEvent.BindUFunction(this, FName("ReloadEvent"));
	ReloadEventTimeline->AddEvent(0.5f, ReloadActionEvent); // Hard Coded Event Time
	ReloadEventTimeline->AddInterpFloat(ReloadEventCurve, ReloadEventTimelineCallback);
	ReloadEventTimeline->SetLooping(false);
	ReloadEventTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

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
	LookInput = InputValue.Get<FVector2D>();
	AddControllerYawInput(LookInput.Y);
	AddControllerPitchInput(-LookInput.X);
}

void AUPlayerCharacter::WeaponSway()
{
	float MaxSwayDegree = 15.0f;
	float LookSwayX = LookInput.X * MaxSwayDegree;
	float LookSwayY = LookInput.Y * MaxSwayDegree;
	FRotator FinalRot = FRotator(-LookSwayX, LookSwayY, LookSwayY);
	FRotator InitialRot;

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FRotator TargetSwayRotator = FRotator
	(
		0,
		(InitialRot.Yaw + Cur_WeaponData->Transform.Rotator().Yaw) + FinalRot.Yaw,
		InitialRot.Pitch + FinalRot.Pitch
	);
	
	float ClampSwayMax = bIsAiming ? MaxSwayDegree/2 : MaxSwayDegree;

	TargetSwayRotator.Roll = FMath::Clamp(TargetSwayRotator.Roll, -ClampSwayMax, ClampSwayMax);
	TargetSwayRotator.Yaw = FMath::Clamp
	(
		TargetSwayRotator.Yaw,
		Cur_WeaponData->Transform.Rotator().Yaw - ClampSwayMax,
		Cur_WeaponData->Transform.Rotator().Yaw + ClampSwayMax
	);

	WeaponMesh->SetRelativeRotation(FMath::RInterpTo(WeaponMesh->GetRelativeRotation(), TargetSwayRotator, DeltaTime, 3));
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

#pragma region Aiming Functions

void AUPlayerCharacter::Aim()
{
	if (!bIsReloading) {
		bIsAiming = true;
		AimAnimationTimeline->Play();
	}
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

#pragma endregion

#pragma region Shooting Functions

void AUPlayerCharacter::Shoot()
{
	if (!Cur_WeaponData)
	{
		return;
	}

	if (AmmoClipCur > 0 && !bIsReloading)
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

void AUPlayerCharacter::AutoFire()
{
	if (AmmoClipCur > 0)
	{
		AActor* HitActor = FiringLineTrace(); // Have to line trace here due to input type
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

void AUPlayerCharacter::ToggleWeaponDelay()
{
	bWeaponDelay = false;
	GetWorld()->GetTimerManager().ClearTimer(WeaponDelayTimerHandle);

}

AActor* AUPlayerCharacter::FiringLineTrace()
{
	if (bIsReloading) {
		return nullptr;
	}

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


#pragma endregion

#pragma region Reload Functions

void AUPlayerCharacter::Reload()
{
	if (CanReload() && !bIsReloading)
	{
		bIsReloading = true;

		StoppedShooting();
		UnAim();

		ReloadAnimationTimeline->SetPlayRate(1 / Cur_WeaponData->ReloadSpeed);
		ReloadEventTimeline->SetPlayRate(1 / Cur_WeaponData->ReloadSpeed);

		ReloadAnimationTimeline->PlayFromStart();
		ReloadEventTimeline->PlayFromStart();
	}
}

void AUPlayerCharacter::UpdateReloadAnimation(float Value)
{
	FRotator TargetRotation = FRotator
	(
		0,
		Cur_WeaponData->Transform.Rotator().Yaw,
		FMath::Lerp(0.0f, 90.0f, Value)
	);

	WeaponMesh->SetRelativeRotation(TargetRotation);
}

void AUPlayerCharacter::ReloadEvent()
{
	if (AmmoTotalCur > Cur_WeaponData->AmmoClipMax - AmmoClipCur)
	{
		AmmoTotalCur -= Cur_WeaponData->AmmoClipMax - AmmoClipCur;
		AmmoClipCur = Cur_WeaponData->AmmoClipMax;
	}
	else
	{
		AmmoClipCur += AmmoTotalCur;
		AmmoTotalCur = 0;
	}

	UpdateAmmo(AmmoClipCur, AmmoTotalCur);
}

void AUPlayerCharacter::ReloadEventTimelineFinished()
{
	bIsReloading = false;
}

bool AUPlayerCharacter::CanReload()
{
	if (AmmoTotalCur > 0 && AmmoClipCur < Cur_WeaponData->AmmoClipMax)
	{
		return true;
	}

	return false;
}

#pragma endregion

#pragma region Recoil Functions

void AUPlayerCharacter::Recoil()
{
	if (RecoilTimeline && RecoilCurve)
	{
		RecoilTimeline->SetPlayRate(1 / Cur_WeaponData->RecoilLength);

		RecoilTimeline->PlayFromStart();
		RecoilAnimationTimeline->PlayFromStart();
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

	RecoilAnimation(Value);

}

void AUPlayerCharacter::RecoilTimelineAnimFinished()
{
	bVariablesSet = false;
	UE_LOG(LogTemp, Warning, TEXT("Recoil Animation Finished!"));
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

void AUPlayerCharacter::SetRecoilVariablesAfterAiming()
{
	PreRecoilRotation = WeaponMesh->GetRelativeRotation().Roll;
	RecoilRotAmount = WeaponMesh->GetRelativeRotation().Roll + -Cur_WeaponData->RecoilAmount;

	PreRecoilLocation = WeaponMesh->GetRelativeLocation().X;
	RecoilLocationAmount = WeaponMesh->GetRelativeLocation().X + -Cur_WeaponData->PullBackAmount;
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
