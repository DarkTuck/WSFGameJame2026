// Fill out your copyright notice in the Description page of Project Settings.


#include "EndTriggerActor.h"

#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"


// Sets default values
AEndTriggerActor::AEndTriggerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SphereComp = CreateDefaultSubobject<USphereComponent>(FName("SphereComp"));
	SphereComp->SetSphereRadius(50);
	RootComponent = SphereComp;
	
}

// Called when the game starts or when spawned
void AEndTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEndTriggerActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
}

void AEndTriggerActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	EndScreenWidget=CreateWidget(PC, EndScreenWidgetClass);
	PC->bShowMouseCursor=true;
	EndScreenWidget->AddToViewport();
}

// Called every frame
void AEndTriggerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

