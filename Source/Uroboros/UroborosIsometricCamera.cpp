// Fill out your copyright notice in the Description page of Project Settings.


#include "UroborosIsometricCamera.h"

UUroborosIsometricCamera::UUroborosIsometricCamera(const FObjectInitializer& ObjectInitializer)
{
	const float Roll = -asin(1/sqrt(3));
	SetRelativeRotation(FRotator(
		0.0f, // Pitch
		Roll, // Roll
		45.f // Yaw
		));
	
}
