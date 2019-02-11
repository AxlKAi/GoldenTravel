// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "MainMenuUIbase.generated.h"

/**
 * 
 */
UCLASS()
class GOLDTRAVEL_02_API UMainMenuUIbase : public UUserWidget
{
	GENERATED_BODY()

public:
		// PlayerState reference
	UPROPERTY(BlueprintReadOnly, Category = "State Manager")
		APlayerStateBase * pStateRef;		
	
	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void OnSinglePlayerClicked();

	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void OnTwoPlayersClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "State Manager")
		void SetExitButtonVisibility(ESlateVisibility newVisibility);

	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void CloseMainMenu();

};
