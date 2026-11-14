// Fill out your copyright notice in the Description page of Project Settings.


#include "Uroboros/Public/UroborosCharacter.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Uroboros/UroborosIsometricCamera.h"


// Sets default values
AUroborosCharacter::AUroborosCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	IsometricCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("IsometricCamera"));
	const float CameraYaw = -asin(1.00/sqrt(3.00));
	IsometricCamera->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 500.0f),FRotator(-90.0f,CameraYaw,45.f));
#if 0
	UStaticMeshComponent* cubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	UStaticMesh* cubeMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")).Object;
	cubeMeshComponent->SetStaticMesh(cubeMesh);
#endif
	
	IsometricCamera->bUsePawnControlRotation = false;
}

void AUroborosCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);	
}

void AUroborosCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

// Called every frame
void AUroborosCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AUroborosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUroborosCharacter::MoveInput);
	}
}

