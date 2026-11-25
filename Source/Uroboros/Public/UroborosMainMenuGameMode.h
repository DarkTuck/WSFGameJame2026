// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UroborosMainMenuGameMode.generated.h"
class UUserWidget;
/**
 * 
 */
UCLASS()
class UROBOROS_API AUroborosMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()
	AUroborosMainMenuGameMode(const FObjectInitializer& ObjectInitializer);
	void BeginPlay() override;
public:
	UPROPERTY(EditAnywhere,Category="MainMenu")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UUserWidget> MainMenuWidget;
};
