// Fill out your copyright notice in the Description page of Project Settings.


#include "UroborosAIControler.h"

#include "EnemyBase.h"
#include "BehaviorTree/BehaviorTree.h"


// Sets default values
AUroborosAIControler::AUroborosAIControler()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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
		if (UBehaviorTree* const npcBehaviorTree = npc->GetBehaviorTree())
		{
			UBlackboardComponent*b;
			UseBlackboard(npcBehaviorTree->BlackboardAsset,b);
			Blackboard=b;
			RunBehaviorTree(npcBehaviorTree);
		}
	}
}

// Called every frame
void AUroborosAIControler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

