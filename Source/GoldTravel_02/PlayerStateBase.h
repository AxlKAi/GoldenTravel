// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameSparks/generated/GSTypes.h"
#include "Private/GSMessageListeners.h"
#include "Private/GameSparksComponent.h"
#include "GameSparksModule.h"
#include <GameSparks/GS.h>
#include <GameSparks/generated/GSResponses.h>
#include <GameSparks/generated/GSRequests.h>

#include "GameFramework/PlayerState.h"

// use this class to store info from Player Friends list
#include "FriendListStruct.h"

#include "PlayerStateBase.generated.h"

// enum of player states
UENUM(BlueprintType)
enum class EGameState : uint8
{
	beginPlay		UMETA(DisplayName = "beginPlay"),
	hideStage		UMETA(DisplayName = "hideStage"),
	findStage		UMETA(DisplayName = "findStage"),
	endPlay			UMETA(DisplayName = "endPlay")
};

class UUserMenuUI;

/**
 * 
 */
UCLASS()
class GOLDTRAVEL_02_API APlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:

	// in this class we initialize GameSparks component
	// and set listeners
	APlayerStateBase(const FObjectInitializer & ObjectInitializer);
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
	virtual void Tick(float DeltaSeconds);


	// -------------- Menus --------------
	
	// To reference Main menu Class 
	// First menu that player see when start the game
	UPROPERTY(EditDefaultsOnly, Category = "State Manager")
		TSubclassOf<class UMainMenuUIbase> mainMenuGameUI;
	
	UPROPERTY(BlueprintReadOnly, Category = "State Manager")
		UMainMenuUIbase * mainMenu;	

	// To reference Login ( Autorization menu ) menu Class 
	UPROPERTY(EditDefaultsOnly, Category = "State Manager")
		TSubclassOf<class UUserMenuUI> cLoginMenu;
	// user widget menu ( Autorization menu )
	// ref to handle created menu
	UPROPERTY(BlueprintReadOnly, Category = "State Manager")
		UUserMenuUI * userMenu;	

	// To reference UI that show players stats in game
	UPROPERTY(EditDefaultsOnly, Category = "State Manager")
		TSubclassOf<class UGameUIbase> cUserGameUI;		
	// ref to handle created menu
	UPROPERTY(BlueprintReadOnly, Category = "State Manager")
		UGameUIbase * gameUI;
	
	//if we close multiplayer menu , we should make Game scene active.
	// Set input mode To game and UI
	void SetInputModeGame();
	
	//change input mode to UI only and show mouse cursor
	void SetInputModeUIonly();

	void ShowMainMenu();
	void ShowUserMenu();	
	void CloseAllMenu();

	// ------------  Single Player --------------

	// call it from mainMenuUI to start single game
	void StartSingleGame();

	// Get Score from APlayerStateBase
	UFUNCTION(BlueprintPure, Category = "State Manager")
		int32 GetPlayerScore() { return playerPoints; }

	// Set new player Score (in local APlayerStateBase), so we shuold show something
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State Manager")
		void SetPlayerScore(int32 newScore);

	// in this param we declare how many chest will be visible
	UPROPERTY(BlueprintReadOnly, Category = "State Manager")
		int32 visiblePairs = singlePlayMaxTurns;
	// event to show or hide chest
	// main logic in BP
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State Manager")
		void ShowChests();

	// TODO i am not shure that i need make it BlueprintCallable
	UFUNCTION(BlueprintCallable, Category = "State Manager")
		//// parse number of player from chest-Scene object name
		// return 1 for player1 
		//        2 for player2
		int32 ParsePlayer(FString chestName);

	// TODO i am not shure that i need make it BlueprintCallable
	UFUNCTION(BlueprintCallable, Category = "State Manager")
		// parse number of chest from chest-Scene object name
		int32 ParseChest(FString chestName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State Manager")
		void OpenChestEvent(const FString &chestName, bool isTreasure);

	UFUNCTION(BlueprintImplementableEvent, Category = "State Manager")
		void ShowTreasureInEnemyChest(int32 chestNum);

	UFUNCTION(BlueprintImplementableEvent, Category = "State Manager")
		void CloseAllChest();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State Manager")
		void CloseChestEvent(const FString &chestName);



	// ------------  MULTIPLAYER --------------
	
	TArray<FriendListItem> PlayerFriendsList;

	// main GameSparks Component ( created in this class )
	UGameSparksComponent* GameSparksComp;

	//Function used to determine what happens if GameSparks connects or fails to (Needs to be UFUNCTION)
	UFUNCTION()
		void OnGameSparksAvailable(bool available);

	// autorize player
	void AutorizePlayer(FString username, FString password);

	// used for WidgetUI text element ( show autorization info )
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State Manager")
		void ShowOnAutorized();

	void RegisterPlayer(FString username, FString password);

	// Get & Set score on GS-server
	void GetScoreGS();
	void SetScoreGS();

	//Get List of friends
	void SendGetFriendsList();

	// Get Score Table from GS-server
	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void GetScoreTable();

	// Call this function from BP , when user click on chest
	UFUNCTION(BlueprintCallable, Category = "State Manager")
		void ChestClicked(FString chestname);

	void FindMatch();

	// We call this event when we recieve signal from GS-server
	// that enemy hide treasure in some pair of chests
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State Manager")
		void EnemyChestHideMessage(int32 pairNum);

	UPROPERTY(BlueprintReadOnly, Category = "State Manager")
		EGameState roundState = EGameState::beginPlay;

	// tru if user Autorized
	// if isUserAutorized = false , than all enemy action provided from AI
	// if isUserAutorized = true , than we recieve/send messages from/to GS-server
	UPROPERTY(BlueprintReadOnly, Category = "State Manager")
		bool isUserAutorized = false;

	UPROPERTY(BlueprintReadOnly, Category = "State Manager")
		bool isPlayerOne;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State Manager")
		void recieveHideStageMessage();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State Manager")
		void recieveFindStageMessage();


private:

	// ------------  Menus --------------

	// Create first menu that player see 
	void CreateMainMenuUI();

	// Create multiplayer Autorization menu
	void CreateUUserMenuUI();

	// Create GameUI widget witch show all Player Statistic in game
	void CreateGameUI();

	
	// ------------  Single Player --------------

	// true only in single game , against AI
	bool isSinglePlayer = false;

	// to run AI move by timer
	FTimerHandle AImoveTimer;
	// change roundState with a little dalay
	FTimerHandle changeRoundStateTimer;
	float roundStateDelay = 1.f;
	// use it to know did i set changeRoundStateTimer in Tick or not 
	bool isTimerSet = false;
	// need newState to send in to function, because cant start functions with parameters in timer
	// so i set this value , and in ChangeRoundStateSingle i make logic by newState
	// but in all other functions a use roundState value
	EGameState newState;

	// how mutch AI and Player have turns at begining of game
	static const int32 singlePlayMaxTurns = 4;
	// how mutch AI turns left 
	int32 AIturnsLeft = 0;
	// how mutch PlAYER turns left 
	int32 playerTurns = 0;

	void AIhideTreasure();
	void AIfindTreasure();
	
	float AIturnDelay = .7f;
	
	enum { EMPTY_CHEST = 0, CHEST_WITH_TREASURE = 1, OPEN_CHEST = 3, OPEN_TREASURE_CHEST = 4 };
	TArray<int32> playerChests;
	TArray<int32> enemyChests;
	
	int32 playerPoints = 0;
	int32 enemyPoints = 0;
	
	void ChangeRoundStateSingle();

	// ------------  MULTIPLAYER --------------
			
	FString challengeID = "";
	FString playerID = "";
	FString playerName = "";
	FString enemyName = "";
	bool isPlayersInChallange = false;

	TArray<bool> clickCheck;
		
	// first Event that we send to GS-server , to initialize Player
	void SendInitPlayerEvent();
	// Send HIDE event to GS-sever
	void SendHideEvent(int32 chestNum);
	// Send FIND event to GS-sever
	void SendFindEvent(int32 chestNum);



	// ------------  OTHER STUFF --------------

	// i check isFirstRun once in SetDefaultValues 
	// to know did i run game first time or not
	// if it a first time i didnt make all values
	// to defaults, because they allready at default values
	bool isFirstRun = true;
	// set all values to default.
	// use it when i select in menu new game
	// i didnt reload level
	void SetDefaultValues();

	// log out array to UE_LOG
	template <typename TArrayType>
	FORCEINLINE void LogOutTArray(const TArray<TArrayType> & TArr, const FString & LogMessage) const
	{
		FString TArrString = FString("[ ");
		for (TArrayType val : TArr)
		{
			TArrString.Append(FString::FromInt((int32)val) + " ");
		}
		TArrString.Append("]");
		UE_LOG(LogTemp, Warning, TEXT("%s - %s"), *LogMessage, *TArrString);
	}
};
