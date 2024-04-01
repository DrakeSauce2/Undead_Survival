// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayUI.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	void SetOwningPlayerCharacter(class AUPlayerCharacter* OwningPlayerCharacter);

	void SetupRoundChangedDelegate(class AUEnemyRoundSpawner* EnemyRoundSpawner);

private:
	UFUNCTION()
		void AmmoUpdated(int32 NewAmmo, int32 NewTotalAmmo);
	UFUNCTION()
		void RoundUpdated(int32 NewRound);
	UFUNCTION()
		void GameOver();
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* AmmoClipCurrentText;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* AmmoTotalText;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* RoundCountText;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* EndText;

	int CurrentRound = 0;
	UPROPERTY()
		class AUPlayerCharacter* PlayerCharacter;


};
