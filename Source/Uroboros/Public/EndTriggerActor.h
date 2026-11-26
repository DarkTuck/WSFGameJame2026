// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EndTriggerActor.generated.h"
class USphereComponent;

UCLASS()
class UROBOROS_API AEndTriggerActor : public AActor
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
	USphereComponent* SphereComp;
	UPROPERTY()
	TObjectPtr<UUserWidget> EndScreenWidget;
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UUserWidget> EndScreenWidgetClass;


public:
	// Sets default values for this actor's properties
	AEndTriggerActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
