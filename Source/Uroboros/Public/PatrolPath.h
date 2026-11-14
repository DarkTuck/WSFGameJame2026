// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolPath.generated.h"

UCLASS()
class UROBOROS_API APatrolPath : public AActor
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI",meta=(MakeEditWidget=true, AllowPrivateAccess=true))
	TArray<FVector> PatrolPoints;
	
public:
	// Sets default values for this actor's properties
	APatrolPath();
	
	FVector GetPathPoint(int const index) const;
	int NUM() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
