// Fill out your copyright notice in the Description page of Project Settings.

#include "GoldTravel_02.h"
#include "PlayerStateBase.h"
#include "UserMenuUI.h"

void UUserMenuUI::NativeConstruct()
{

}

void UUserMenuUI::OnLoginClicked()
{
	if (pStateRef == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find PlayerState, please select right Class for it"));
	}
	else
	{
		pStateRef->AutorizePlayer(username, password);
	}
}

void UUserMenuUI::OnRegistrationClicked()
{
	if (pStateRef == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find PlayerState, please select right Class for it"));
	}
	else
	{
		pStateRef->RegisterPlayer(username, password);
	}
}

void UUserMenuUI::OnGetScoreClicked()
{
	pStateRef->GetScoreGS();
}

void UUserMenuUI::OnSetScoreClicked(int32 newScore)
{
	pStateRef->SetPlayerScore(newScore);
	pStateRef->SetScoreGS();
}

void UUserMenuUI::OnHightScoreClicked()
{
	pStateRef->GetScoreTable();
}

void UUserMenuUI::OnStartMatchClicked()
{
	pStateRef->FindMatch();
}

void UUserMenuUI::OnCloseMultiplayerMenuClicked()
{
	pStateRef->ShowMainMenu();
}

void UUserMenuUI::OnGetFriendsClicked()
{
	pStateRef->SendGetFriendsList();
}