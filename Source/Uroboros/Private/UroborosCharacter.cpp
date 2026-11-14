// Fill out your copyright notice in the Description page of Project Settings.


#include "Uroboros/Public/UroborosCharacter.h"
#include "Uroboros/UroborosIsometricCamera.h"


// Sets default values
AUroborosCharacter::AUroborosCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	IsometricCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("IsometricCamera"));
	const float CameraYaw = -asin(1/sqrt(3));
	IsometricCamera->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 500.0f),FRotator(-90.0f,CameraYaw,45.f));
#if WITH_EDITORONLY_DATA
	UStaticMeshComponent* cubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	UStaticMesh* cubeMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")).Object;
	cubeMeshComponent->SetStaticMesh(cubeMesh);
#endif
	
	IsometricCamera->bUsePawnControlRotation = false;
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

