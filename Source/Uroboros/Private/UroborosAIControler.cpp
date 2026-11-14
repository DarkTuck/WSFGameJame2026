// Fill out your copyright notice in the Description page of Project Settings.


#include "UroborosAIControler.h"

#include "EnemyBase.h"
#include "UroborosCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"


// Sets default values
AUroborosAIControler::AUroborosAIControler()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// create the AI perception component. It will be configured in BP
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	// subscribe to the AI perception delegates
	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AUroborosAIControler::OnPerceptionUpdated);
	AIPerception->OnTargetPerceptionForgotten.AddDynamic(this, &AUroborosAIControler::OnPerceptionForgotten);
}

// Called when the game starts or when spawned
void AUroborosAIControler::BeginPlay()
{
	Super::BeginPlay();
}

void AUroborosAIControler::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (AEnemyBase* const npc= Cast<AEnemyBase>(InPawn))
	{
		EnemyOwner=npc;
		if (UBehaviorTree* const npcBehaviorTree = npc->GetBehaviorTree())
		{
			UBlackboardComponent*b;
			UseBlackboard(npcBehaviorTree->BlackboardAsset,b);
			Blackboard=b;
			RunBehaviorTree(npcBehaviorTree);
		}
	}
}

void AUroborosAIControler::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Actor->Tags.Contains(PlayerTag)&&Stimulus.WasSuccessfullySensed())
	{
		Blackboard->SetValueAsBool(SeesPlayerKeyName, true);
#if WITH_EDITOR
		GEngine->AddOnScreenDebugMessage(0,1.f,FColor::Green,TEXT("Seen Player"));
#endif
		if (AUroborosCharacter* const PlayerCharacter = Cast<AUroborosCharacter>(Actor))
		{
			PlayerCharacter->StartSpawningStone();
		}
#if WITH_EDITOR
		else
		{
			GEngine->AddOnScreenDebugMessage(0,1.f,FColor::Red,TEXT("Failed To Cast Player Class"));
		}
#endif
	}
}

void AUroborosAIControler::OnPerceptionForgotten(AActor* Actor)
{
	Blackboard->SetValueAsBool(SeesPlayerKeyName, false);
}

// Called every frame
void AUroborosAIControler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

