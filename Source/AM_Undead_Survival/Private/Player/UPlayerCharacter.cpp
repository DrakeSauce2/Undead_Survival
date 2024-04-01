#include "Player/UPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/DecalComponent.h"

#include "DrawDebugHelpers.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "Engine/World.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/BlueprintCore.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include "GameFramework/CharacterMovementComponent.h"

AUPlayerCharacter::AUPlayerCharacter()
{
	ViewCamera = CreateDefaultSubobject<UCameraComponent>("View Camera");
	ViewCamera->SetupAttachment(GetRootComponent());

	FPSMesh = CreateDefaultSubobject<USkeletalMeshComponent>("FPS Mesh");
	FPSMesh->SetupAttachment(ViewCamera);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	WeaponMesh->SetupAttachment(FPSMesh);


	RecoilTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RecoilTimeline"));
	RecoilAnimationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RecoilAnimationTimeline"));
	AimAnimationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AimAnimationTimeline"));
	ReloadAnimationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ReloadAnimationTimeline"));
	ReloadEventTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ReloadEventTimeline"));
	EquipAnimationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("EquipAnimationTimeline"));

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
	SetMaxAmmo();
	
	Cur_WeaponData->FPSMeshTransform = FPSMesh->GetRelativeTransform();

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

	FOnTimelineFloat EquipAnimTimelineCallback;
	EquipAnimTimelineCallback.BindUFunction(this, FName("EquipWeaponAnimation"));
	EquipAnimationTimeline->AddInterpFloat(EquipAnimCurve, EquipAnimTimelineCallback);
	EquipAnimationTimeline->SetTimelineFinishedFunc(FOnTimelineEventStatic::CreateUFunction(this, FName("EquipAnimationFinished")));

}

void AUPlayerCharacter::SwitchWeaponSlot(const FInputActionValue& InputValue, int SlotNumber)
{
	if (WeaponSlots[SlotNumber].WeaponIndex > 0 && WeaponSlotInUse != WeaponSlots[SlotNumber].WeaponIndex && !bIsSwapping) {
		bIsSwapping = true;

		EquipAnimationTimeline->SetPlayRate(1.5f);

		WeaponSlotInUse = SlotNumber;

		WeaponSlots[WeaponIndex].AmmoClip = AmmoClipCur;
		WeaponSlots[WeaponIndex].AmmoTotal = AmmoTotalCur;

		StoppedShooting();
		UnAim();

		NewWeaponSlotNumber = SlotNumber;

		UnEquip();
	}
}

void AUPlayerCharacter::SetWeaponSlot(int SlotNumber)
{
	WeaponIndex = WeaponSlots[SlotNumber].WeaponIndex;
	AmmoClipCur = WeaponSlots[SlotNumber].AmmoClip;
	AmmoTotalCur = WeaponSlots[SlotNumber].AmmoTotal;

	SetWeaponStats();
}

void AUPlayerCharacter::SetMaxAmmo()
{
	AmmoClipCur = Cur_WeaponData->AmmoClipMax;
	AmmoTotalCur = Cur_WeaponData->AmmoTotalMax - Cur_WeaponData->AmmoClipMax;

	UpdateAmmo(AmmoClipCur, AmmoTotalCur);
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

		enhancedInputComp->BindAction(WeaponSlotOneInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::SwitchWeaponSlot, 1);
		enhancedInputComp->BindAction(WeaponSlotTwoInputAction, ETriggerEvent::Triggered, this, &AUPlayerCharacter::SwitchWeaponSlot, 2);

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
	if (!bIsAiming || !bIsReloading || !bIsSwapping)
	{
		float MaxSwayDegree = 15.0f;
		float LookSwayX = LookInput.X * MaxSwayDegree;
		float LookSwayY = LookInput.Y * MaxSwayDegree;
		FRotator FinalRot = FRotator(-LookSwayX, LookSwayY, LookSwayY);

		FRotator InitialRot;

		float DeltaTime = GetWorld()->GetDeltaSeconds();
		FRotator TargetSwayRotator = FRotator
		(
			InitialRot.Pitch + FinalRot.Pitch,
			(InitialRot.Yaw + Cur_WeaponData->FPSMeshTransform.Rotator().Yaw) + FinalRot.Yaw,
			0
		);
		TargetSwayRotator.Pitch = FMath::Clamp(-TargetSwayRotator.Pitch, -3, 3);
		TargetSwayRotator.Yaw = FMath::Clamp(TargetSwayRotator.Yaw, -8, 8);

		FRotator TargetRotator = FMath::RInterpTo(
			FPSMesh->GetRelativeRotation(),
			FRotator(TargetSwayRotator.Pitch, TargetSwayRotator.Yaw, 0),
			DeltaTime,
			3.0f
		);

		FPSMesh->SetRelativeRotation(FMath::RInterpTo(FPSMesh->GetRelativeRotation(), TargetSwayRotator, DeltaTime, 3));
	}



}

void AUPlayerCharacter::EquipWeaponAnimation(float Value)
{
	FVector TargetLocation = FVector
	(
		Cur_WeaponData->FPSMeshTransform.GetLocation().X,
		Cur_WeaponData->FPSMeshTransform.GetLocation().Y,
		FMath::Lerp(Cur_WeaponData->FPSMeshTransform.GetLocation().Z - 50, Cur_WeaponData->FPSMeshTransform.GetLocation().Z, Value)
	);


	FTransform TargetTransform = FTransform(Cur_WeaponData->FPSMeshTransform.GetRotation(), TargetLocation, FVector::OneVector);
	FPSMesh->SetRelativeTransform(TargetTransform);
}

void AUPlayerCharacter::Equip()
{
	EquipAnimationTimeline->PlayFromStart();
}

void AUPlayerCharacter::UnEquip()
{
	EquipAnimationTimeline->ReverseFromEnd();
}

void AUPlayerCharacter::EquipAnimationFinished()
{
	float PlaybackPosition = EquipAnimationTimeline->GetPlaybackPosition();

	if (PlaybackPosition == 1) {
		bIsSwapping = false;
	}

	if (PlaybackPosition == 0) {
		SetWeaponSlot(WeaponSlotInUse);
		Equip();
	}

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
			WeaponMesh->AttachToComponent(FPSMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Gun_Socket"));


			UpdateAmmo(AmmoClipCur, AmmoTotalCur);
		}
	}
}

#pragma region Aiming Functions

void AUPlayerCharacter::Aim()
{
	if (!bIsReloading && !bIsSwapping) {
		bIsAiming = true;
		AimAnimationTimeline->Play();
		GetWorld()->GetTimerManager().ClearTimer(WeaponSwayTimerHandle);
	}
}

void AUPlayerCharacter::UnAim()
{
	bIsAiming = false;
	GetWorld()->GetTimerManager().SetTimer(WeaponSwayTimerHandle, this, &AUPlayerCharacter::WeaponSway, WeaponSwayRate, true);
	AimAnimationTimeline->Reverse();
}

FTransform AUPlayerCharacter::CalculateAimTransform() const
{
	FTransform WeaponMeshSocketTransform = WeaponMesh->GetSocketTransform(FName("Aim_Socket"), ERelativeTransformSpace::RTS_World);
	FTransform FPSMeshWorldTransform = FPSMesh->GetComponentTransform();

	return UKismetMathLibrary::InvertTransform(UKismetMathLibrary::MakeRelativeTransform(WeaponMeshSocketTransform, FPSMeshWorldTransform));
}

void AUPlayerCharacter::UpdateAimAnimation(float Value)
{
	FVector LerpedLocation = FMath::Lerp(Cur_WeaponData->FPSMeshTransform.GetLocation(), CalculateAimTransform().GetLocation(), Value);
	FQuat LerpedRotation = FQuat::Slerp(Cur_WeaponData->FPSMeshTransform.GetRotation(), CalculateAimTransform().GetRotation(), Value);
	FVector LerpedScale = FMath::Lerp(Cur_WeaponData->FPSMeshTransform.GetScale3D(), CalculateAimTransform().GetScale3D(), Value);

	FTransform TargetTransform = FTransform(LerpedRotation, LerpedLocation, LerpedScale);
	FPSMesh->SetRelativeTransform(TargetTransform);

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

	if (AmmoClipCur > 0 && !bIsReloading && !bIsSwapping)
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

				HandleDamage(HitActor);

				bWeaponDelay = true;
			}

			if (!WeaponDelayTimerHandle.IsValid())
			{
				GetWorld()->GetTimerManager().SetTimer(WeaponDelayTimerHandle, this, &AUPlayerCharacter::ToggleWeaponDelay, WeaponDelayRate, false, Cur_WeaponData->FireRate);
			}

			return;
		}
	}
}

void AUPlayerCharacter::HandleDamage(AActor* TargetActor)
{
	if (TargetActor)
	{
		AUCharacterBase* Target = Cast<AUCharacterBase>(TargetActor);
		if (Target) {
			Target->TakeDamage(Cur_WeaponData->Damage);
		}
	}
}

void AUPlayerCharacter::AutoFire()
{
	if (AmmoClipCur > 0)
	{
		AActor* HitActor = FiringLineTrace(); // Have to line trace here due to input type

		HandleDamage(HitActor);
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
	if (bIsReloading || bIsSwapping) {
		return nullptr;
	}

	float BulletSpread = 0.0f;
	if (!bIsAiming) {
		BulletSpread = -Cur_WeaponData->BulletSpread;
	}

	FName SocketName = FName("Muzzle");
	UGameplayStatics::SpawnEmitterAttached(
		Cur_WeaponData->MuzzleFlash, 
		WeaponMesh,  
		SocketName, 
		FVector::ZeroVector, 
		Cur_WeaponData->MuzzleRotation,  
		EAttachLocation::SnapToTargetIncludingScale, 
		true  
	);

	UGameplayStatics::SpawnSoundAttached(
		Cur_WeaponData->FireSound,
		WeaponMesh,  
		SocketName,  
		FVector::ZeroVector, 
		Cur_WeaponData->MuzzleRotation,
		EAttachLocation::SnapToTarget,  
		true  
	);

	FVector StartLocation = ViewCamera->GetComponentLocation();
	FVector ForwardEndVector = ViewCamera->GetForwardVector() * Cur_WeaponData->FireMaxRange;
	FVector RightEndVector = ViewCamera->GetRightVector() * FMath::RandRange(-BulletSpread, BulletSpread);
	FVector UpEndVector = ViewCamera->GetUpVector() * FMath::RandRange(-BulletSpread, BulletSpread);
	FVector TargetEndVector = ForwardEndVector + RightEndVector + UpEndVector;

	FVector EndLocation = TargetEndVector;
	ECollisionChannel TraceChannel = ECC_Visibility;
	FCollisionQueryParams CollisionParams;
	CollisionParams.bTraceComplex = true;
	CollisionParams.AddIgnoredActor(this->GetOwner());

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
			DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Red, false, 3.0f, 0);

			DecrementClipAmmo();

			Recoil();

			FVector XVector = FVector::CrossProduct(HitResult.Normal, FVector::UpVector);
			XVector.Normalize();
			FRotator TargetRotation = UKismetMathLibrary::MakeRotFromX(XVector);

			FTransform TargetTransform = FTransform(TargetRotation, HitResult.ImpactPoint, FVector::OneVector);

			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				Cur_WeaponData->ImpactParticle,
				TargetTransform
			);
			
			UGameplayStatics::SpawnDecalAttached(
				Cur_WeaponData->BulletHoleDecal,
				FVector(5.0f, 5.0f, 5.0f),
				HitResult.GetComponent(),
				HitResult.BoneName,
				HitResult.Location,
				FRotator(TargetRotation.Pitch, TargetRotation.Yaw, FMath::RandRange(0, 360)),
				EAttachLocation::KeepRelativeOffset,
				60.0f
			);

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
	if (CanReload() && !bIsReloading && !bIsSwapping)
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
	float PreReloadHeight = FPSMesh->GetRelativeLocation().Z;

	FVector TargetLocation = FVector
	(
		FPSMesh->GetRelativeLocation().X,
		FPSMesh->GetRelativeLocation().Y,
		FMath::Lerp(PreReloadHeight, PreReloadHeight - 5.0f, Value)
	);

	FPSMesh->SetRelativeLocation(TargetLocation);
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

	int RecoilModifier = bIsAiming ? 1 : 2;
	PreRecoilRotation = FPSMesh->GetRelativeRotation().Pitch;
	RecoilRotAmount = FPSMesh->GetRelativeRotation().Pitch + -Cur_WeaponData->RecoilAmount * RecoilModifier;

	RecoilModifier = bIsAiming ? -0.5 : -1;
	PreRecoilLocation = FPSMesh->GetRelativeLocation().X;
	RecoilLocationAmount = FPSMesh->GetRelativeLocation().X + -Cur_WeaponData->PullBackAmount * RecoilModifier;

	bVariablesSet = true;
}

void AUPlayerCharacter::SetRecoilVariablesAfterAiming()
{
	int RecoilModifier = bIsAiming ? 1 : 2;
	PreRecoilRotation = FPSMesh->GetRelativeRotation().Pitch;
	RecoilRotAmount = FPSMesh->GetRelativeRotation().Pitch + -Cur_WeaponData->RecoilAmount * RecoilModifier;

	RecoilModifier = bIsAiming ? -0.5 : -1;
	PreRecoilLocation = FPSMesh->GetRelativeLocation().X;
	RecoilLocationAmount = FPSMesh->GetRelativeLocation().X + -Cur_WeaponData->PullBackAmount * RecoilModifier;

	FPSMeshTransformAim = FPSMesh->GetRelativeTransform();
}

void AUPlayerCharacter::RecoilAnimation(float Alpha)
{
	FRotator TargetRotation = FRotator
	(
		FMath::Lerp(PreRecoilRotation, RecoilRotAmount, Alpha),
		FPSMesh->GetRelativeRotation().Yaw,
		FPSMesh->GetRelativeRotation().Roll
	);
	FPSMesh->SetRelativeRotation(TargetRotation);

	FVector TargetLocation = FVector
	(
		FMath::Lerp(PreRecoilLocation, RecoilLocationAmount, Alpha),
		FPSMesh->GetRelativeLocation().Y,
		FPSMesh->GetRelativeLocation().Z
	);
	FPSMesh->SetRelativeLocation(TargetLocation);
}

#pragma endregion
