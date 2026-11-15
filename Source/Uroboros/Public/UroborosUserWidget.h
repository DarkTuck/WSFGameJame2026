// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UroborosUserWidget.generated.h"

/**
 * 
 */
UCLASS(abstract)
class UROBOROS_API UUroborosUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, Category="LevelID")
	void BP_UpdateLevelUI(int32 Level);
};
