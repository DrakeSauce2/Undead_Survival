// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UEnemyRoundSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundChanged, int32, CurrentRound);

UCLASS()
class AUEnemyRoundSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUEnemyRoundSpawner();

	FOnRoundChanged OnRoundChanged;

	void EnemyCountUpdated();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void StartNextRound();
	void UpdateCurrentRound();
	void SpawnEnemy();

	int CalculateEnemiesPerRound();
	float CalculateEnemyHealth();

	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
		int MaxEnemySpawnCount = 10;
	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
		TSubclassOf<AActor> EnemyToSpawn;
	UPROPERTY(EditAnywhere, Category = "Spawner")
		TArray<AActor*> SpawnPoints;

	float RoundChangeDuration = 3.0f;
	FTimerHandle RoundChangeTimer;
	FTimerHandle ActiveRoundTimer;

	float BaseHealth = 150.0f;

	int EnemiesSpawned = 0;
	int EnemyLiveCount = 0;
	int RoundNumber = 0;

};
