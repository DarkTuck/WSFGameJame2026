// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelSettingsActor.h"


// Sets default values
ALevelSettingsActor::ALevelSettingsActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevelSettingsActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALevelSettingsActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

