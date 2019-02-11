// Fill out your copyright notice in the Description page of Project Settings.

#include "GoldTravel_02.h"
#include "PlayerStateBase.h"
#include "MainMenuUIbase.h"

void UMainMenuUIbase::OnSinglePlayerClicked()
{
	pStateRef->CloseAllMenu();
	pStateRef->StartSingleGame();
}


void UMainMenuUIbase::OnTwoPlayersClicked()
{
	pStateRef->ShowUserMenu();
}

void UMainMenuUIbase::CloseMainMenu()
{
	pStateRef->CloseAllMenu();
}


