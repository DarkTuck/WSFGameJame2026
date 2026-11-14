// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelEndActor.generated.h"

class UBoxComponent;

UCLASS()
class UROBOROS_API ALevelEndActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevelEndActor();
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* StaticMesh;
	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* BoxCollision;
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* StaticMeshComp;
	UPROPERTY(EditAnywhere)
	FName LevelToLoad;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
