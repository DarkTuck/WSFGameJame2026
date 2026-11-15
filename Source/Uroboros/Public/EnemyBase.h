// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class UBehaviorTree;

UCLASS()
class UROBOROS_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI",meta=(AllowPrivateAccess=true))
	UBehaviorTree* BehaviorTree;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UStaticMeshComponent* CubeMesh;
#endif

public:
	// Sets default values for this character's properties
	AEnemyBase();
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; };
	
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Line of Sight", meta = (AllowPrivateAccess = "true"))
	class UProceduralMeshComponent * LOSMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Line of Sight", meta = (AllowPrivateAccess = "true"))
	UMaterial* LOSMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Line of Sight", meta = (AllowPrivateAccess = "true", UIMin = "1.0", UIMax = "360.0"))
	float ArcAngle;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Line of Sight", meta = (AllowPrivateAccess = "true", UIMin = "1.0", UIMax = "5.0"))
	float AngleStep;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Line of Sight", meta = (AllowPrivateAccess = "true", UIMin = "200", UIMax = "1000"))
	float Radius;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Line of Sight", meta = (AllowPrivateAccess = "true"))
	TArray<FVector> LOSVertices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Line of Sight", meta = (AllowPrivateAccess = "true"))
	TArray<int32> LOSTriangles;

private:

	void InitLOSMesh();

	void TickLOSMesh(float DeltaSeconds);

	void UpdateLOSMeshData(const TArray<FVector>& Vertices, const TArray<int32>& Triangles);
};
