// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UroborosCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UROBOROS_API UUroborosCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float DashStrength=5000.f;
	
public:
	// Sets default values for this component's properties
	UUroborosCharacterMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void StartDash(const float Right, const float Forward);
	void StopDash();
	void PerformDash(const float Right, const float Forward);
};
