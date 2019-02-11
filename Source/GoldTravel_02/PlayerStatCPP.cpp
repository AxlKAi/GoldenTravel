// Fill out your copyright notice in the Description page of Project Settings.

#include "GoldTravel_02.h"
#include "PlayerStatCPP.h"


// Sets default values
APlayerStatCPP::APlayerStatCPP()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayerStatCPP::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerStatCPP::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void APlayerStatCPP::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

