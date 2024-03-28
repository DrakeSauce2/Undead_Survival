// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GameplayUI.h"

#include "Components/TextBlock.h"

#include "Player/UPlayerCharacter.h"


void UGameplayUI::NativeConstruct()
{
	Super::NativeConstruct();

}

void UGameplayUI::SetOwningPlayerCharacter(AUPlayerCharacter* OwningPlayerCharacter)
{
	PlayerCharacter = OwningPlayerCharacter;
	if (PlayerCharacter) {
		PlayerCharacter->OnAmmoChanged.AddDynamic(this, &UGameplayUI::AmmoUpdated);
	}
}

void UGameplayUI::AmmoUpdated(int32 NewAmmo, int32 NewTotalAmmo)
{
	FText NewAmmoText = FText::Format(FText::FromString("{0}"), FText::AsNumber((int)NewAmmo));
	AmmoClipCurrentText->SetText(NewAmmoText);

	FText NewTotalAmmoText = FText::Format(FText::FromString("{0}"), FText::AsNumber((int)NewTotalAmmo));
	AmmoTotalText->SetText(NewAmmoText);
}
