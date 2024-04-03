// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/UEnemyRoundSpawner.h"

#include "Character/UCharacterBase.h"

#include "Engine/World.h"

#include "Widgets/GameplayUI.h"

#include "Kismet/GameplayStatics.h"

#include "Player/UPlayerController.h"

// Sets default values
AUEnemyRoundSpawner::AUEnemyRoundSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUEnemyRoundSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* Player = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		AUPlayerController* PlayerCharacter = Cast<AUPlayerController>(Player);
		if (PlayerCharacter) {
			PlayerCharacter->GetGameplayUI()->SetupRoundChangedDelegate(this);
		}
	}

	StartNextRound();
}

void AUEnemyRoundSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUEnemyRoundSpawner::StartNextRound()
{
	if (RoundChangeTimer.IsValid()) 
	{
		return;
	}

	if (ActiveRoundTimer.IsValid()) 
	{
		GetWorld()->GetTimerManager().ClearTimer(ActiveRoundTimer);
		EnemiesSpawned = 0;
	}

	UE_LOG(LogTemp, Warning, TEXT("Round Starting!"));
	RoundNumber++;

	OnRoundChanged.Broadcast(RoundNumber);

	BaseHealth = CalculateEnemyHealth();

	GetWorld()->GetTimerManager().SetTimer(ActiveRoundTimer, this, &AUEnemyRoundSpawner::UpdateCurrentRound, 1.0f, true);

}

void AUEnemyRoundSpawner::UpdateCurrentRound()
{
	if (EnemiesSpawned >= CalculateEnemiesPerRound() && EnemyLiveCount == 0) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Round Ended!"));

		StartNextRound();

		return;
	}

	if (EnemyLiveCount < MaxEnemySpawnCount && EnemiesSpawned < CalculateEnemiesPerRound()) {
		SpawnEnemy();
	}
}

void AUEnemyRoundSpawner::SpawnEnemy()
{
	if (EnemyToSpawn) 
	{
		int SpawnLocation = FMath::RandRange(0, SpawnPoints.Num() - 1);
		FActorSpawnParameters* SpawnParams = new FActorSpawnParameters();
		SpawnParams->SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams->Owner = this;
		SpawnParams->Instigator = GetInstigator();

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(EnemyToSpawn, SpawnPoints[SpawnLocation]->GetActorLocation(), SpawnPoints[SpawnLocation]->GetActorRotation(), *SpawnParams);
		if (SpawnedActor) 
		{
			AUCharacterBase* EnemyCharacter = Cast<AUCharacterBase>(SpawnedActor);
			if (EnemyCharacter) {
				EnemyCharacter->SetHealth(BaseHealth);
				EnemyCharacter->OnDead.AddUObject(this, &AUEnemyRoundSpawner::EnemyCountUpdated);
			}
		}

		delete SpawnParams;

		EnemiesSpawned++;
		EnemyLiveCount++;
	}
}

int AUEnemyRoundSpawner::CalculateEnemiesPerRound()
{
	int TotalEnemies = 0;
	if(RoundNumber < 10)
	{
		float Multiplier = RoundNumber / 2;
		TotalEnemies = MaxEnemySpawnCount + FMath::RoundToInt(Multiplier);
		return TotalEnemies;
	}
	else
	{
		float Multiplier = RoundNumber * 0.15f;
		TotalEnemies = FMath::RoundToInt(MaxEnemySpawnCount * Multiplier);
		return TotalEnemies;
	}
}

float AUEnemyRoundSpawner::CalculateEnemyHealth()
{
	float EnemyHealth = 0.0f;

	if (RoundNumber < 10)
	{
		EnemyHealth = (BaseHealth + 100.0f) - 100.0f;
		return EnemyHealth;
	}
	else
	{
		EnemyHealth = BaseHealth * 1.1f;
		return EnemyHealth;
	}
}

void AUEnemyRoundSpawner::EnemyCountUpdated()
{
	EnemyLiveCount--;
	UE_LOG(LogTemp, Warning, TEXT("Enemy Count Updated, %d Enemies Left!"), EnemyLiveCount);
}


