// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "GameUIbase.generated.h"

class APlayerStateBase;
/**
 * 
 */
UCLASS()
class GOLDTRAVEL_02_API UGameUIbase : public UUserWidget
{
	GENERATED_BODY()

public:
	// PlayerState reference
	UPROPERTY(BlueprintReadOnly, Category = "State Manager")
		APlayerStateBase * pStateRef;

	UFUNCTION(BlueprintImplementableEvent, Category = "State Manager")
		void ShowPlayerName(const FString & playerName);

	UFUNCTION(BlueprintImplementableEvent, Category = "State Manager")
		void ShowPlayerPoints(const int32 & playerPoints);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "State Manager")
		void ShowEnemyPoints(const int32 & enemyPoints);

	UFUNCTION(BlueprintImplementableEvent, Category = "State Manager")
		void ShowRoundStatus(const FString & playerName);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "State Manager")
		void ShowWinnerText(const FString & playerName );

	// right now in-game-menu and main-menu is the same
	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void ShowGameMenu();
	
private:
	
};
