// Fill out your copyright notice in the Description page of Project Settings.


#include "UroborosMainMenuGameMode.h"

#include "Blueprint/UserWidget.h"

AUroborosMainMenuGameMode::AUroborosMainMenuGameMode(const FObjectInitializer& ObjectInitializer)
{
	DefaultPawnClass=nullptr;
}

void AUroborosMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	MainMenuWidget = CreateWidget(PC, MainMenuWidgetClass);
	PC->bShowMouseCursor=true;
	MainMenuWidget->AddToViewport();
}
