// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UEnemyRoundSpawner.generated.h"

UCLASS()
class AUEnemyRoundSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUEnemyRoundSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
		TArray<TSubclassOf<class AUCharacterBase>> SpawnList;

	int EnemyLiveCount = 0;
	int RoundNumber = 1;

};
