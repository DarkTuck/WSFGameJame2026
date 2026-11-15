// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelSettingsActor.generated.h"

UCLASS()
class UROBOROS_API ALevelSettingsActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevelSettingsActor();

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	int PlayerLives=3;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	int LevelID=0;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintPure)
	int GetLevelID() const {return LevelID;};
};
