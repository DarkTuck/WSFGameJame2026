// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "UroborosAIControler.generated.h"

class AEnemyBase;
struct FAIStimulus;

UCLASS()
class UROBOROS_API AUroborosAIControler : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AUroborosAIControler();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerception;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	FName SeesPlayerKeyName = FName("SeePlayer");
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	FName PlayerTag = FName("Player");

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	UFUNCTION()
	virtual void OnPerceptionForgotten(AActor* Actor);
	
private:
	UPROPERTY()
	AEnemyBase* EnemyOwner;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
