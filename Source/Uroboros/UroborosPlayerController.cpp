// Fill out your copyright notice in the Description page of Project Settings.


#include "UroborosPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "LevelSettingsActor.h"
#include "UroborosUserWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AUroborosPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void AUroborosPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UIWidget = CreateWidget<UUroborosUserWidget>(this, UIWidgetClass);
	if (!UIWidget)
		return;
	UIWidget->AddToPlayerScreen(0);
	if (const ALevelSettingsActor* LevelSetting = Cast<ALevelSettingsActor>(UGameplayStatics::GetActorOfClass(GetWorld(),ALevelSettingsActor::StaticClass())))
	{
		UIWidget->BP_UpdateLevelUI(LevelSetting->LevelID);
	}
}
