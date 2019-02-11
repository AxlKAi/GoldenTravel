// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "UserMenuUI.generated.h"

class APlayerStateBase;
/**
 * 
 */
UCLASS()
class GOLDTRAVEL_02_API UUserMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:

	// PlayerState reference
	UPROPERTY(BlueprintReadOnly, Category = "State Manager")
		APlayerStateBase * pStateRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State Manager")
		FString username;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State Manager")
		FString password;

	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void OnLoginClicked();

	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void OnRegistrationClicked();

	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void OnGetScoreClicked();

	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void OnSetScoreClicked(int32 newScore);

	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void OnHightScoreClicked();

	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void OnStartMatchClicked();

	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void OnCloseMultiplayerMenuClicked();

	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void OnGetFriendsClicked();

private:
	// Optionally override the Blueprint "Event Construct" event
	virtual void NativeConstruct() override;

};
