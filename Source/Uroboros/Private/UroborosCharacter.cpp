// Fill out your copyright notice in the Description page of Project Settings.


#include "Uroboros/Public/UroborosCharacter.h"
#include "Uroboros/UroborosIsometricCamera.h"


// Sets default values
AUroborosCharacter::AUroborosCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	IsometricCamera = CreateDefaultSubobject<UUroborosIsometricCamera>(TEXT("IsometricCamera"));
}

// Called when the game starts or when spawned
void AUroborosCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUroborosCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AUroborosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

