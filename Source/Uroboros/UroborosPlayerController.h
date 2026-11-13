// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UroborosPlayerController.generated.h"

class UInputMappingContext;
/**
 * 
 */
UCLASS()
class UROBOROS_API AUroborosPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;
	
	
	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

};
