// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelEndActor.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ALevelEndActor::ALevelEndActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMesh = CreateDefaultSubobject<UStaticMesh>(TEXT("StaticMesh"));
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("StaticMeshComponent"));
	StaticMeshComp->SetStaticMesh(StaticMesh);
	RootComponent = StaticMeshComp;
	
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(RootComponent);
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ALevelEndActor::OnBoxBeginOverlap);
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxCollision->SetCollisionProfileName(FName("OverlapAll"));
}

// Called when the game starts or when spawned
void ALevelEndActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALevelEndActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelEndActor::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
#if WITH_EDITOR
	GEngine->AddOnScreenDebugMessage(5,1.f,FColor::Green,TEXT("OnBoxBeginOverlap"));
#endif
	
	if (OtherActor->Tags.Contains(FName("Player")))
	{
		if (LevelToLoad!="")
		{
#if WITH_EDITOR
			GEngine->AddOnScreenDebugMessage(5,1.f,FColor::Green,TEXT("LevelNotEmpty"));
#endif
			UGameplayStatics::OpenLevel(GetWorld(),LevelToLoad);
		}
	}
}

