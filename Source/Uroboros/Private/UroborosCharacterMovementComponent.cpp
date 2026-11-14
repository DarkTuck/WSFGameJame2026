// Fill out your copyright notice in the Description page of Project Settings.


#include "UroborosCharacterMovementComponent.h"


// Sets default values for this component's properties
UUroborosCharacterMovementComponent::UUroborosCharacterMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUroborosCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UUroborosCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUroborosCharacterMovementComponent::StartDash(const float Right, const float Forward)
{
#if WITH_EDITOR
	GEngine->AddOnScreenDebugMessage(0,1.f,FColor::Cyan,TEXT("StartDash"));
#endif
	PerformDash(Right,Forward);
	
}

void UUroborosCharacterMovementComponent::StopDash()
{
#if WITH_EDITOR
	GEngine->AddOnScreenDebugMessage(0,1.f,FColor::Cyan,TEXT("StopDash"));
#endif
}

void UUroborosCharacterMovementComponent::PerformDash(const float Right, const float Forward)
{
	FVector DashDirection = (Acceleration.IsNearlyZero() ? UpdatedComponent->GetForwardVector() : Acceleration).GetSafeNormal2D();
	//	FVector DashDirection = StoredInput.GetSafeNormal2D();
	DashDirection += GetOwner()->GetActorRightVector()*Right;
	DashDirection += GetOwner()->GetActorForwardVector()*Forward;
	//	DashDirection += StoredInput.GetSafeNormal2D();
	Velocity = DashDirection*DashStrength;

	const FQuat NewRotation = FRotationMatrix::MakeFromXZ(DashDirection,FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector,NewRotation,false,Hit);
}

