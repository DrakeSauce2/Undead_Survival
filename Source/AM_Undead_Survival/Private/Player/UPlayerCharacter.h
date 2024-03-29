// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Character/UCharacterBase.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"

#include "Engine/DataTable.h"

#include "UPlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class FireType : uint8 
{
	SingleFire,
	Automatic
};

USTRUCT(BlueprintType)
struct FUWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		class USkeletalMesh* TargetWeaponMesh;

	UPROPERTY(EditAnywhere)
		FTransform Transform;

	UPROPERTY(EditAnywhere)
		FireType FiringType;

	UPROPERTY(EditAnywhere)
		float RecoilAmount;
	UPROPERTY(EditAnywhere)
		float RecoilYaw;
	UPROPERTY(EditAnywhere)
		float RecoilLength;
	UPROPERTY(EditAnywhere)
		float FireRate;
	UPROPERTY(EditAnywhere)
		float BulletSpread;
	UPROPERTY(EditAnywhere)
		float FireMaxRange;
	UPROPERTY(EditAnywhere)
		float PullBackAmount;
	UPROPERTY(EditAnywhere)
		float AimFOV;
	UPROPERTY(EditAnywhere)
		int32 AmmoClipMax;
	UPROPERTY(EditAnywhere)
		int32 AmmoTotalMax;
	UPROPERTY(EditAnywhere)
		float ReloadSpeed;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAmmoChangedDelegate, int32, CurrentAmmo, int32, TotalAmmo);

class UCameraComponent;
class USkeletalMeshComponent;
class UInputMappingContext;
class UInputAction;
class UDataTable;
/**
 * 
 */
UCLASS()
class AUPlayerCharacter : public AUCharacterBase
{
	GENERATED_BODY()
	
public:
	//this is the construtor
	AUPlayerCharacter();

	FAmmoChangedDelegate OnAmmoChanged;

	void UpdateAmmo(int32 NewCurrentAmmo, int32 NewTotalAmmo);

protected:
	virtual void BeginPlay() override;

private:


	UPROPERTY(visibleAnywhere, Category = "View")
		UCameraComponent* ViewCamera;

	UPROPERTY(visibleAnywhere, Category = "Weapon")
		USkeletalMeshComponent* WeaponMesh;



	/*****************************************************/
	/*                       Input                       */
	/*****************************************************/
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputMappingContext* inputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* LookInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* JumpInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* ReloadInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* ShootInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
		UInputAction* AimInputAction;

	UFUNCTION()
		void Move(const FInputActionValue& InputValue);

	UFUNCTION()
		void Look(const FInputActionValue& InputValue);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*****************************************************/
	/*                      Weapon                       */
	/*****************************************************/

	/*
	* I need to find a better way, other than timelines
	*/
	UFUNCTION()
		void InitializeTimelines(); 

	UFUNCTION()
		void Shoot();
	UFUNCTION()
		void DecrementClipAmmo();
	UFUNCTION()
		void StoppedShooting();
	UFUNCTION()
		void Aim();
	UFUNCTION()
		void UnAim();
	UFUNCTION()
		void Reload();
	bool CanReload();


	UFUNCTION()
		AActor* FiringLineTrace();
	UFUNCTION()
		void SetWeaponStats();
	UFUNCTION()
		void AutoFire();
	UFUNCTION()
		void ToggleWeaponDelay();
	

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		int32 WeaponIndex;
	UPROPERTY(EditAnywhere)
		int32 AmmoClipCur;
	UPROPERTY(EditAnywhere)
		int32 AmmoTotalCur;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		UDataTable* WeaponDataTable;

	FUWeaponData* Cur_WeaponData;

	UPROPERTY(EditAnywhere)
		float PreRecoilRotation;
	UPROPERTY(EditAnywhere)
		float RecoilRotAmount;
	UPROPERTY(EditAnywhere)
		float PreRecoilLocation;
	UPROPERTY(EditAnywhere)
		float RecoilLocationAmount;
	UPROPERTY()
		FVector WeaponStartingLocation;

	UFUNCTION()
		void Recoil();
	UFUNCTION()
		void SetRecoilVariables();
	UFUNCTION()
		void SetRecoilVariablesAfterAiming();
	UFUNCTION()
		void RecoilAnimation(float Alpha);
	UFUNCTION()
		void UpdateRecoil(float Value);
	UFUNCTION()
		void UpdateRecoilAnimation(float Value);
	UFUNCTION()
		void RecoilTimelineAnimFinished();
	UFUNCTION()
		void UpdateAimAnimation(float Value);
	UFUNCTION()
		void UpdateReloadAnimation(float Value);
	UFUNCTION()
		void ReloadEvent();
	UFUNCTION()
		void ReloadEventTimelineFinished();
	UFUNCTION()
		void WeaponSway();

	UPROPERTY()
		UTimelineComponent* RecoilTimeline;
	UPROPERTY()
		UTimelineComponent* RecoilAnimationTimeline;
	UPROPERTY()
		UTimelineComponent* AimAnimationTimeline;
	UPROPERTY()
		UTimelineComponent* ReloadAnimationTimeline;
	UPROPERTY()
		UTimelineComponent* ReloadEventTimeline;

	UPROPERTY(EditDefaultsOnly, Category = "Float Curve")
		UCurveFloat* RecoilCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Float Curve")
		UCurveFloat* RecoilAnimCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Float Curve")
		UCurveFloat* AimAnimCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Float Curve")
		UCurveFloat* ReloadAnimCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Float Curve")
		UCurveFloat* ReloadEventCurve;

	float WeaponDelayRate = 1.0f/60.0f;
	float WeaponSwayRate = 1.0f/60.0f;
	FTimerHandle WeaponDelayTimerHandle;
	FTimerHandle AutoFireTimerHandle;
	FTimerHandle WeaponSwayTimerHandle;

	FVector2D LookInput;

	bool bIsReloading = false;
	bool bIsAiming = false;
	bool bWeaponDelay = false;
	bool bVariablesSet = false;

};
