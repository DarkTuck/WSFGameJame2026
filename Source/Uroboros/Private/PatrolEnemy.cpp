// Fill out your copyright notice in the Description page of Project Settings.


#include "PatrolEnemy.h"


// Sets default values
APatrolEnemy::APatrolEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APatrolEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APatrolEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APatrolEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

