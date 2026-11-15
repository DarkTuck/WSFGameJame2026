// Fill out your copyright notice in the Description page of Project Settings.


#include "Uroboros/Public/UroborosCharacter.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "LevelSettingsActor.h"
#include "UroborosCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Uroboros/UroborosIsometricCamera.h"


// Sets default values
AUroborosCharacter::AUroborosCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UUroborosCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	IsometricCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("IsometricCamera"));
	const float CameraYaw = -asin(1.00/sqrt(3.00));
	IsometricCamera->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 500.0f),FRotator(-90.0f,CameraYaw,45.f));
	IsometricCamera->SetupAttachment(GetCapsuleComponent());
	UroborosCharacterMovement = Cast<UUroborosCharacterMovementComponent>(GetCharacterMovement());
	
	
	Tags.Add(TEXT("Player"));
#if 0
	UStaticMeshComponent* cubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	UStaticMesh* cubeMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")).Object;
	cubeMeshComponent->SetStaticMesh(cubeMesh);
#endif
	
	IsometricCamera->bUsePawnControlRotation = false;
}

void AUroborosCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);	
}

void AUroborosCharacter::DoMove(float Right, float Forward)
{
	LastInputRight = Right;
	LastInputForward = Forward;
	
	if (Right>0)
	{
		GetMesh()->SetRelativeRotation(FRotator(0,90,0));
	}
	else if (Right<0)
	{
		GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
	}
	if (Forward>0)
	{
		GetMesh()->SetRelativeRotation(FRotator(0,0,0));
	}
	if (Forward<0)
	{
		GetMesh()->SetRelativeRotation(FRotator(0,270,0));
	}
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AUroborosCharacter::Dash()
{
	UroborosCharacterMovement->StartDash(LastInputRight,LastInputForward);
}

void AUroborosCharacter::StopDash()
{
	UroborosCharacterMovement->StopDash();
}

// Called every frame
void AUroborosCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AUroborosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUroborosCharacter::MoveInput);
		EnhancedInputComponent->BindAction(DashAction,ETriggerEvent::Started,this,&AUroborosCharacter::Dash);
		EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Completed,this,&AUroborosCharacter::StopDash);
	}
}

void AUroborosCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (const ALevelSettingsActor* LevelSetting = Cast<ALevelSettingsActor>(UGameplayStatics::GetActorOfClass(GetWorld(),ALevelSettingsActor::StaticClass())))
	{
#if WITH_EDITOR
		GEngine->AddOnScreenDebugMessage(10,2.f,FColor::Green,TEXT("Changing lives amount"));
#endif
		
		Lives = LevelSetting->PlayerLives;
	}
#if WITH_EDITOR
	else
	{
		GEngine->AddOnScreenDebugMessage(10,5.f,FColor::Red,TEXT("No Level Settings Found Falling back to default amount of lives"));	
	}
#endif
	
	CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
}

void AUroborosCharacter::StartSpawningStone()
{
	if (bSpawningStone)
		return;
	bSpawningStone = true;
	GetWorld()->GetTimerManager().SetTimer(StoneTimerHandle,this,&AUroborosCharacter::SpawnStone,StoneSpawnDelay,false);
}

void AUroborosCharacter::SpawnStone()
{
	GetWorld()->GetTimerManager().ClearTimer(StoneTimerHandle);
#if WITH_EDITOR
	GEngine->AddOnScreenDebugMessage(2,1.f,FColor::Yellow,TEXT("SpawningStone"));
#endif
	// spawn the projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	SpawnParams.Owner = GetOwner();
	
	const FTransform SpawnTransform = GetCharacterMovement()->GetActorTransform();
		
	GetWorld()->SpawnActor(Stone, &SpawnTransform, SpawnParams);
	GetWorld()->GetTimerManager().SetTimer(RespawnDelayTimer,this,&AUroborosCharacter::OnRespawnDelayTimer,RespawnDelay);
}

void AUroborosCharacter::OnRespawnDelayTimer()
{
	RespawnCharacter();
	GetWorld()->GetTimerManager().ClearTimer(RespawnDelayTimer);
	bSpawningStone=false;
}

void AUroborosCharacter::RespawnCharacter()
{
	--Lives;
	if (Lives<=0)
	{
		UGameplayStatics::OpenLevel(GetWorld(),*CurrentLevelName);
		return;
	}

	SetActorLocation(GetOwner()->GetActorLocation());
}

