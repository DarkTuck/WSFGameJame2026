// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UroborosCharacter.generated.h"

class UUroborosCharacterMovementComponent;
class UInputAction;
struct FInputActionValue;
class UCameraComponent;

UCLASS()
class UROBOROS_API AUroborosCharacter : public ACharacter
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UCameraComponent* IsometricCamera;
	
	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* DashAction;
	
	UPROPERTY(EditAnywhere, Category = "Stone")
	TSubclassOf<AActor> Stone;
	
	UPROPERTY(EditAnywhere, Category = "Stone")
	int Lives = 3;
	
	UPROPERTY(EditAnywhere, Category = "Stone")
	float StoneSpawnDelay = 1.f;
	UPROPERTY(EditAnywhere, Category = "Stone")
	float RespawnDelay = 1.f;
	
	FTimerHandle StoneTimerHandle;
	FTimerHandle RespawnDelayTimer;
	bool bSpawningStone = false;
	bool bLockMove = false;
private:
	UPROPERTY()
	UUroborosCharacterMovementComponent* UroborosCharacterMovement;
	
	float LastInputRight=0;
	float LastInputForward=0;

	FString CurrentLevelName;
	
public:
	// Sets default values for this character's properties
	AUroborosCharacter(const FObjectInitializer& ObjectInitializer);
private:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UStaticMeshComponent* CubeMesh;
#endif

protected:
	
	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);
	
	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);
	UFUNCTION(BlueprintCallable, Category="Input")
	void Dash();
	UFUNCTION(BlueprintCallable, Category="Input")
	void StopDash();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	
	void StartSpawningStone();	
	virtual void SpawnStone();
	
	void OnRespawnDelayTimer();
	
	void RespawnCharacter();
};
