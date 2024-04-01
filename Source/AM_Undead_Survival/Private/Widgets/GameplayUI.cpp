// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GameplayUI.h"

#include "AI/UEnemyRoundSpawner.h"

#include "Components/TextBlock.h"

#include "Player/UPlayerCharacter.h"


void UGameplayUI::NativeConstruct()
{
	Super::NativeConstruct();

	EndText->SetVisibility(ESlateVisibility::Hidden);
}

void UGameplayUI::SetOwningPlayerCharacter(AUPlayerCharacter* OwningPlayerCharacter)
{
	PlayerCharacter = OwningPlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerCharacter->OnAmmoChanged.AddDynamic(this, &UGameplayUI::AmmoUpdated);
		PlayerCharacter->OnPlayerDead.AddUObject(this, &UGameplayUI::GameOver);
	}
}

void UGameplayUI::SetupRoundChangedDelegate(AUEnemyRoundSpawner* EnemyRoundSpawner)
{
	if (EnemyRoundSpawner)
	{
		EnemyRoundSpawner->OnRoundChanged.AddDynamic(this, &UGameplayUI::RoundUpdated);
	}
}

void UGameplayUI::AmmoUpdated(int32 NewAmmo, int32 NewTotalAmmo)
{
	FText NewAmmoText = FText::Format(FText::FromString("{0}"), FText::AsNumber((int)NewAmmo));
	AmmoClipCurrentText->SetText(NewAmmoText);

	FText NewTotalAmmoText = FText::Format(FText::FromString("{0}"), FText::AsNumber((int)NewTotalAmmo));
	AmmoTotalText->SetText(NewTotalAmmoText);
}

void UGameplayUI::RoundUpdated(int32 NewRound)
{
	FText NewRoundCountText = FText::Format(FText::FromString("Round: {0}"), FText::AsNumber((int)NewRound));
	RoundCountText->SetText(NewRoundCountText);

	CurrentRound = NewRound;
}

void UGameplayUI::GameOver()
{
	EndText->SetVisibility(ESlateVisibility::Visible);
	FText GameOverText = FText::Format(FText::FromString("You Survived {0} Rounds"), FText::AsNumber((int)CurrentRound));
	EndText->SetText(GameOverText);
}
