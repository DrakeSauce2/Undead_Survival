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

	void SetOwningPlayerCharacter(class AUPlayerCharacter* OwningPlayerCharacter); // his is my short term solution(Hopefully), Since I'm bad at coding

private:
	UFUNCTION()
		void AmmoUpdated(int32 NewAmmo, int32 NewTotalAmmo);
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* AmmoClipCurrentText;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* AmmoTotalText;
	UPROPERTY()
		class AUPlayerCharacter* PlayerCharacter;


};
