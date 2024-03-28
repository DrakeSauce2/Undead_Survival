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

	void UpdateAmmo(int32 NewAmmo, int32 NewTotalAmmo);

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


	UFUNCTION()
		void Reload();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*****************************************************/
	/*                      Weapon                       */
	/*****************************************************/

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
		void RecoilAnimation(float Alpha);
	UFUNCTION()
		void UpdateRecoil(float Value);
	UFUNCTION()
		void UpdateRecoilAnimation(float Value);
	UFUNCTION()
		void RecoilTimelineAnimFinished();
	UFUNCTION()
		void UpdateAimAnimation(float Value);

	UPROPERTY()
		UTimelineComponent* RecoilTimeline;
	UPROPERTY()
		UTimelineComponent* RecoilAnimationTimeline;
	UPROPERTY()
		UTimelineComponent* AimAnimationTimeline;

	UPROPERTY(EditDefaultsOnly, Category = "Float Curve")
		UCurveFloat* RecoilCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Float Curve")
		UCurveFloat* RecoilAnimCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Float Curve")
		UCurveFloat* AimAnimCurve;

	float WeaponDelayRate = 1.0f;
	FTimerHandle WeaponDelayTimerHandle;
	FTimerHandle AutoFireTimerHandle;

	bool bIsAiming = false;
	bool bWeaponDelay = false;
	bool bVariablesSet = false;

};
