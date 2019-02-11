// Fill out your copyright notice in the Description page of Project Settings.

#include "GoldTravel_02.h"
#include "UserMenuUI.h"
#include "MainMenuUIbase.h"
#include "GameUIbase.h"
#include "PlayerStateBase.h"

APlayerStateBase::APlayerStateBase(const FObjectInitializer & ObjectInitializer) : APlayerState(ObjectInitializer)
{
	clickCheck.Init(false, 5);

	//Initialize GameSparksComponent used to connect
	GameSparksComp = CreateDefaultSubobject<UGameSparksComponent>(TEXT("GSparkComp"));

	GameSparksComp->OnGameSparksAvailableDelegate.AddDynamic(this, &APlayerStateBase::OnGameSparksAvailable);

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerStateBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::BeginPlay"))

	CreateMainMenuUI();
	CreateUUserMenuUI();
	CreateGameUI();
	SetInputModeUIonly();
	ShowMainMenu();

	gameUI->ShowRoundStatus("Start");
}

void APlayerStateBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// if we are in Single Player mode
	if (isSinglePlayer)
	{
		if (playerTurns == 0 && AIturnsLeft == 0 && roundState == EGameState::hideStage && !isTimerSet)
		{
			newState = EGameState::findStage;
			GetWorld()->GetTimerManager().SetTimer(changeRoundStateTimer, this, &APlayerStateBase::ChangeRoundStateSingle, roundStateDelay, false);	
			isTimerSet = true;
		}
		else if (playerTurns == 0 && AIturnsLeft == 0 && roundState == EGameState::findStage && !isTimerSet)
		{
			if (playerPoints == singlePlayMaxTurns * 2)
			{
				gameUI->ShowWinnerText("Player - WINNER !!");
				newState = EGameState::endPlay;
				GetWorld()->GetTimerManager().SetTimer(changeRoundStateTimer, this, &APlayerStateBase::ChangeRoundStateSingle, .1f, false);				
				isTimerSet = true;
			}
			else if (enemyPoints == singlePlayMaxTurns * 2)
			{
				gameUI->ShowWinnerText("PC - WINNER !!");
				newState = EGameState::endPlay;
				GetWorld()->GetTimerManager().SetTimer(changeRoundStateTimer, this, &APlayerStateBase::ChangeRoundStateSingle, .1f, false);
				isTimerSet = true;
			}
			else
			{
				newState = EGameState::hideStage;
				GetWorld()->GetTimerManager().SetTimer(changeRoundStateTimer, this, &APlayerStateBase::ChangeRoundStateSingle, roundStateDelay, false);
				isTimerSet = true;
			}
		}
	}
}

void APlayerStateBase::CreateGameUI()
{
	gameUI = CreateWidget<UGameUIbase>(GetWorld()->GetFirstPlayerController(), cUserGameUI);
	gameUI->AddToViewport();
	gameUI->pStateRef = this;
}

void APlayerStateBase::CreateMainMenuUI()
{
	mainMenu = CreateWidget<UMainMenuUIbase>(GetWorld()->GetFirstPlayerController(), mainMenuGameUI);
	mainMenu->SetVisibility(ESlateVisibility::Hidden);
	mainMenu->AddToViewport();
	mainMenu->pStateRef = this;
}

void APlayerStateBase::CreateUUserMenuUI()
{
	userMenu = CreateWidget<UUserMenuUI>(GetWorld()->GetFirstPlayerController(), cLoginMenu);
	userMenu->SetVisibility(ESlateVisibility::Hidden);
	userMenu->AddToViewport();
	userMenu->pStateRef = this;
}

void APlayerStateBase::EndPlay(EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::EndPlay"))
	GameSparksComp->Disconnect();
}

void APlayerStateBase::SetInputModeGame()
{
	FInputModeGameAndUI mode;
	userMenu->SetVisibility(ESlateVisibility::Hidden);
	GetWorld()->GetFirstPlayerController()->SetInputMode(mode);
}

void APlayerStateBase::SetInputModeUIonly()
{
	FInputModeUIOnly mode;
	GetWorld()->GetFirstPlayerController()->SetInputMode(mode);
	GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;
}

void APlayerStateBase::ShowMainMenu()
{
	// if we open Main Menu while we playing a game , 
	// we should show exit ( return ) button.
	// Otherwise , whin we in main menu just after starting a game
	// we should not do this
	if (isSinglePlayer || isUserAutorized)
	{
		mainMenu->SetExitButtonVisibility(ESlateVisibility::Visible);
	}
	else
	{
		mainMenu->SetExitButtonVisibility(ESlateVisibility::Hidden);
	}

	userMenu->SetVisibility(ESlateVisibility::Hidden);
	mainMenu->SetVisibility(ESlateVisibility::Visible);
}

void APlayerStateBase::ShowUserMenu()
{
	SetDefaultValues();
	mainMenu->SetVisibility(ESlateVisibility::Hidden);
	userMenu->SetVisibility(ESlateVisibility::Visible);	
}

void APlayerStateBase::CloseAllMenu()
{
	mainMenu->SetVisibility(ESlateVisibility::Hidden);
	userMenu->SetVisibility(ESlateVisibility::Hidden);
	SetInputModeGame();
}

void APlayerStateBase::SetDefaultValues()
{
	if (!isFirstRun)
	{

		GameSparksComp->Disconnect();

		visiblePairs = singlePlayMaxTurns;
		ShowChests();
		CloseAllChest();

		playerPoints = 0;
		enemyPoints = 0;
		challengeID = "";
		playerID = "";
		playerName = "";
		enemyName = "";
		isPlayersInChallange = false;
		isSinglePlayer = false;
		
		GetWorld()->GetTimerManager().ClearTimer(AImoveTimer);
		GetWorld()->GetTimerManager().ClearTimer(changeRoundStateTimer);

		mainMenu->SetExitButtonVisibility(ESlateVisibility::Hidden);

		roundState = newState = EGameState::beginPlay;
		isUserAutorized = false;
		isPlayerOne = false; 
		isTimerSet = false;

		clickCheck.Init(false, 5);
				
	}
	else
	{
		isFirstRun = false;
	}
}


void APlayerStateBase::AutorizePlayer(FString username, FString password)
{
	//make sure our username and password aren't empty
	if (username.IsEmpty() || password.IsEmpty())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::AutorizePlayer %s %s"), *username, *password)

	//Disconnected the module just incase it's connected (Refresh)
	GameSparksComp->Disconnect();
	//Connect to GameSparks using Key and Secret
	GameSparksComp->Connect("T350020co2FL", "VdeRmDaUGBkAAR4MxatUPfh8f16VzmVB");

}

void APlayerStateBase::OnGameSparksAvailable(bool available)
{
	if (available)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, TEXT("Trying to connect whis GameSparks server..."));
		UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::OnGameSparksAvailable"))

		// TODO to mutch call for UGameSparksModule::GetModulePtr()->GetGSInstance()
		GameSparks::Core::GS& gs = UGameSparksModule::GetModulePtr()->GetGSInstance();

		GameSparks::Api::Requests::AuthenticationRequest authRequest(gs);
		authRequest.SetUserName(std::string(TCHAR_TO_UTF8(*(userMenu->username))));
		authRequest.SetPassword(std::string(TCHAR_TO_UTF8(*(userMenu->password))));

		authRequest.Send([&](GameSparks::Core::GS& gs, const GameSparks::Api::Responses::AuthenticationResponse& response) 
		// callback Autorize func
		{
			UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::OnGameSparksAvailable CALLBACK"))
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, response.GetJSONString().c_str());

			playerID = response.GetUserId().GetValue().c_str();
			playerName = response.GetDisplayName().GetValue().c_str();

			//Check is response has no errors
			if (!response.GetHasErrors()) {
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Auth response successful"));

				// attach all MESSAGE LISTENERS in here

				// attach the message listeners (temp) for High Score
				// NewHighScoreMessage
				gs.SetMessageListener<GameSparks::Api::Messages::NewHighScoreMessage>([&](GS& gsi, const GameSparks::Api::Messages::NewHighScoreMessage& message)
				{
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, TEXT("NewHighScoreMessage"));
					UE_LOG(LogTemp, Warning, TEXT("NewHighScoreMessage callback event"))
				});

				// Lisstener for ChallengeStart
				// when it comeing, we start game between two players
				gs.SetMessageListener<GameSparks::Api::Messages::ChallengeStartedMessage>([&](GS& gsi, const GameSparks::Api::Messages::ChallengeStartedMessage& message)
				{
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, TEXT("ChallengeStartedMessage"));
					UE_LOG(LogTemp, Warning, TEXT("ChallengeStartedMessage callback event"))

						challengeID = message.GetChallenge().GetChallengeId().GetValue().c_str();
					UE_LOG(LogTemp, Warning, TEXT("ChallengeID : %s "), *challengeID)

						isPlayersInChallange = true;


					FString challangerID = message.GetChallenge().GetChallenger().GetId().GetValue().c_str();
					if (challangerID == playerID)
					{
						isPlayerOne = true;
						GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("This is Player1"));
						UE_LOG(LogTemp, Warning, TEXT("This is Player1"))
							enemyName = message.GetChallenge().GetChallenged()[0].GetName().GetValue().c_str();
						UE_LOG(LogTemp, Warning, TEXT("Enemy name : %s"), *enemyName)
					}
					else
					{
						isPlayerOne = false;
						GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("This is Player2"));
						UE_LOG(LogTemp, Warning, TEXT("This is Player2"))
							enemyName = message.GetChallenge().GetChallenger().GetName().GetValue().c_str();
						UE_LOG(LogTemp, Warning, TEXT("Enemy name : %s"), *enemyName)
					}
					SendInitPlayerEvent();
				});

				// Listener for ChestClicked from another player
				gs.SetMessageListener<GameSparks::Api::Messages::ScriptMessage>([&](GS& gsi, const GameSparks::Api::Messages::ScriptMessage& message)
				{
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, TEXT("ScriptMessage callback event"));
					UE_LOG(LogTemp, Warning, TEXT("ScriptMessage callback event"))

					FString JSONmess = message.GetJSONString().c_str();
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, JSONmess);
					UE_LOG(LogTemp, Warning, TEXT("ScriptMessage : %s "), *JSONmess)

						// is this message from that challange ? in no than didnt do anything
						if (message.GetData().GetValue().ContainsKey("challengeID"))
						{
							FString strID = message.GetData().GetValue().GetString("challengeID").GetValue().c_str();
							if (strID != challengeID)
							{
								UE_LOG(LogTemp, Error, TEXT("ScriptMessage Error: Wrong challenge ID. ID in game is %s , ID in message is %s"), *challengeID, *strID)
									return;
							}
						}

					if (message.GetData().GetValue().ContainsKey("messageType"))
					{
						int32 mType = message.GetData().GetValue().GetInt("messageType").GetValue();

						switch (mType)
						{
							// sendBeginPlay
						case 0:
							break;

							// sendHideStage
						case 1:
							roundState = EGameState::hideStage;


							for (int32 ii = 0; ii < clickCheck.Num(); ii++)
							{
								clickCheck[ii] = false;
							}

							UE_LOG(LogTemp, Warning, TEXT("ScriptMessage : Starting HideStage"))
								// in chestPairsCount we recieve how many chest will be visible
								if (message.GetData().GetValue().ContainsKey("chestPairsCount"))
								{
									visiblePairs = message.GetData().GetValue().GetInt("chestPairsCount").GetValue();
									UE_LOG(LogTemp, Warning, TEXT("ScriptMessage : Starting HideStage, playing with %d pairs of chests."), visiblePairs)
										ShowChests();
									gameUI->ShowRoundStatus("Hide");
									CloseAllChest();
								}
								else
								{
									UE_LOG(LogTemp, Warning, TEXT("ScriptMessage : Starting HideStage error! We didnt recieve cout of visible chests-pair"))
								}
							break;

							// sendFindStage
						case 2:
							roundState = EGameState::findStage;

							for (int32 ii = 0; ii < clickCheck.Num(); ii++)
							{
								clickCheck[ii] = false;
							}

							UE_LOG(LogTemp, Warning, TEXT("ScriptMessage : Starting FindStage"))
								gameUI->ShowRoundStatus("Find");
							CloseAllChest();
							break;

							// sendClosingPairMessage
						case 3:
							if (message.GetData().GetValue().ContainsKey("closingPair"))
							{
								int32 pairNum = message.GetData().GetValue().GetInt("closingPair").GetValue();
								UE_LOG(LogTemp, Warning, TEXT("Parse pair num : %d "), pairNum)

									if (roundState == EGameState::hideStage)
									{
										EnemyChestHideMessage(pairNum);  // call event to BP/CPP
									}
							}
							break;

							// SendEnemyCheckResult
							//		messageType = 4;
							//		playerPoints - total player Points;
							//		chestNum - number of chest that enemy open;
							//		isTreasure - 1 or 0
						case 4:
							if (message.GetData().GetValue().ContainsKey("playerPoints"))
							{
								int32 pPoints = message.GetData().GetValue().GetInt("playerPoints").GetValue();
								int32 pTreasure = message.GetData().GetValue().GetInt("isTreasure").GetValue();
								int32 pChestNum = message.GetData().GetValue().GetInt("chestNum").GetValue();
								UE_LOG(LogTemp, Warning, TEXT("ScriptMessage : SendPlayerWinPoint : enemy try to guess treasure. total player points: %d , isTreasure %d , chestNum %d"), pPoints, pTreasure, pChestNum)

									FString chestString = isPlayerOne ? FString("Chest_1_" + FString::FromInt(pChestNum)) : FString("Chest_2_" + FString::FromInt(pChestNum));
								UE_LOG(LogTemp, Warning, TEXT("ScriptMessage : SendPlayerWinPoint : chestString %s"), *chestString)

									if (pTreasure == 1)
									{
										playerPoints--;
										enemyPoints++;
										gameUI->ShowPlayerPoints(playerPoints);
										gameUI->ShowEnemyPoints(enemyPoints);

										OpenChestEvent(chestString, true);
									}
									else
									{
										OpenChestEvent(chestString, false);
									}
							}
							else
							{
								UE_LOG(LogTemp, Error, TEXT("ScriptMessage : SendPlayerWinPoint : cant find key 'playerPoints' in script"))
							}
							break;

						case 5:
							if (message.GetData().GetValue().ContainsKey("winner"))
							{
								FString winnerID = message.GetData().GetValue().GetString("winner").GetValue().c_str();

								UE_LOG(LogTemp, Warning, TEXT("ScriptMessage : SendWinnerMessage : We have a winner ! "))
									if (playerID == winnerID)
									{
										UE_LOG(LogTemp, Warning, TEXT("ScriptMessage : SendWinnerMessage : Winner is player %s ! "), *playerName)
											gameUI->ShowWinnerText(playerName);
									}
									else
									{
										UE_LOG(LogTemp, Warning, TEXT("ScriptMessage : SendWinnerMessage : Winner is player %s ! "), *enemyName)
											gameUI->ShowWinnerText(enemyName);
									}
							}
							else
							{
								UE_LOG(LogTemp, Error, TEXT("ScriptMessage : SendWinnerMessage : Cant recognize ID in message. "))
							}
							break;

						default:
							break;
						}
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Messages::ScriptMessage callback : Parse JSON error "))
					}

				});

				// TODO temp
				roundState = EGameState::hideStage;

				isUserAutorized = true;
				//send Event to blueprinst , to show autorized info
				ShowOnAutorized();

				gameUI->ShowPlayerName(playerName);

				// GameSparks::Core::GS& gsIns = UGameSparksModule::GetModulePtr()->GetGSInstance();
				GameSparks::Api::Requests::AccountDetailsRequest accDetRequest(gs);
				//If no errors then send an accounts details request
				accDetRequest.Send([&](GameSparks::Core::GS& gsI, const GameSparks::Api::Responses::AccountDetailsResponse & resp) 
				{
					//check for response errors
					if (!resp.GetHasErrors())
					{
						UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::AutorizationDetailedResponse SUCCES connection response CALLBACK"))
						GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::MakeRandomColor(), resp.GetJSONString().c_str());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::AutorizationDetailedResponse there was some errors in connection"))
						GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::MakeRandomColor(), resp.GetJSONString().c_str());
					}
				});
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("error connecting GameSparks server..."));
			}
		});
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::OnGameSparksAvailable NOT AVALIABLE"))
	}
}

void APlayerStateBase::ShowOnAutorized_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::ShowOnAutorized_Implementation"))
}

void APlayerStateBase::RegisterPlayer(FString username, FString password)
{
	//make sure our username and password aren't empty
	if (username.IsEmpty() || password.IsEmpty())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::RegisterPlayer Trying to register user"))

	//Disconnected the module just incase it's connected (Refresh)
	GameSparksComp->Disconnect();
	//Connect to GameSparks using Key and Secret
	GameSparksComp->Connect("T350020co2FL", "VdeRmDaUGBkAAR4MxatUPfh8f16VzmVB");
	
	//get instance of gamesparks
	GameSparks::Core::GS& gs = UGameSparksModule::GetModulePtr()->GetGSInstance();

	//build our registration request
	GameSparks::Api::Requests::RegistrationRequest req(gs);

	//set the username and password on our request
	req.SetUserName(std::string(TCHAR_TO_UTF8(*username)));
	req.SetDisplayName(std::string(TCHAR_TO_UTF8(*username)));
	req.SetPassword(std::string(TCHAR_TO_UTF8(*password)));

	//send the request
	req.Send([&](GameSparks::Core::GS& gsI, const GameSparks::Api::Responses::RegistrationResponse & resp)
	{
		//check to see if there's been an error
		if (!resp.GetHasErrors())
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::MakeRandomColor(), TEXT("Registration success"));
			UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::RegisterPlayer callback ...Registration success..."))
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::MakeRandomColor(), resp.GetJSONString().c_str());
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::MakeRandomColor(), TEXT("Registration UNsuccess"));
			UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::RegisterPlayer callback ...Registration UNsuccess..."))
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::MakeRandomColor(), resp.GetJSONString().c_str());
		}
	});
}

void APlayerStateBase::GetScoreGS()
{
	if (isUserAutorized)
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::GetScoreGS sending request to get score"))
		//get instance of gamesparks
		GameSparks::Core::GS& gs = UGameSparksModule::GetModulePtr()->GetGSInstance();
		GameSparks::Api::Requests::LogEventRequest request(gs);
		request.SetEventKey("GET_GS_SCORE");

		request.Send([&](GameSparks::Core::GS& gs, const GameSparks::Api::Responses::LogEventResponse & response)
		{
			//check for errors
			if (response.GetHasErrors())
			{
				UE_LOG(LogTemp, Error, TEXT("APlayerStateBase::GetScoreGS callback : some errors when we get score !"))
			}
			else
			{
				
				//If data exists, then do something with it. GetBaseData function extracts the data from the response.
				if (response.GetBaseData().GetGSDataObject("scriptData").GetValue().GetFloat("SCORE").HasValue())
				{
					UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::GetScoreGS callback : get score "))
					SetPlayerScore( response.GetBaseData().GetGSDataObject("scriptData").GetValue().GetFloat("SCORE").GetValue() );
				}
			}
		});
	}
}

void APlayerStateBase::SetPlayerScore_Implementation(int32 newScore)
{ 
	playerPoints = newScore;
}

void APlayerStateBase::SetScoreGS()
{
	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SetScoreGS() write score to GS server "))
	GameSparks::Core::GS& gs = UGameSparksModule::GetModulePtr()->GetGSInstance();
	GameSparks::Api::Requests::LogEventRequest request(gs);
	request.SetEventKey("SET_GS_SCORE");

	request.SetEventAttribute("SCORE", playerPoints);

	request.Send([&](GameSparks::Core::GS& gs, const GameSparks::Api::Responses::LogEventResponse & response)
	{
		//check for errors
		if (response.GetHasErrors())
		{
			UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SetScoreGS() callback : some error in writing score "))
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::MakeRandomColor(), TEXT("error in Writing SCORE"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SetScoreGS() callback : score wrote on GS server"))
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::MakeRandomColor(), TEXT("Writing SCORE"));
		}
	});
}

void APlayerStateBase::GetScoreTable()
{
	if (isUserAutorized)
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::GetScoreTable trying get score table from GS server "))
		//GS Instance reference
		GameSparks::Core::GS& gs = UGameSparksModule::GetModulePtr()->GetGSInstance();

		//Create request instance
		GameSparks::Api::Requests::LeaderboardDataRequest leaderboardRequestInstance(gs);
		//Set Shortcode
		leaderboardRequestInstance.SetLeaderboardShortCode("HIGH_SCORE");
		//Set max entry count (max entries returned)
		leaderboardRequestInstance.SetEntryCount(2);
		//Send request and reference response listener function
		leaderboardRequestInstance.Send([&](GameSparks::Core::GS&, const GameSparks::Api::Responses::LeaderboardDataResponse& response)
		{
			//Check if response is error free
			if (!response.GetHasErrors()) {
				UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::GetScoreTable callback : get score table from GS server "))
				//For every entry returned in Data object (Entries object)
				for (int i = 0; i < response.GetData().size(); i++) {
					//SCORE is the ShortCode for the event attribute linked to this Leaderboard, this will be unique to your event
					FFloat16 score = response.GetData()[i].GetBaseData().GetFloat("SCORE").GetValue();
					//Get player name
					std::string playerName = response.GetData()[i].GetUserNameW().GetValue();
					//Get rank in Leaderboard
					int rank = response.GetData()[i].GetRank().GetValue();
					//Print results
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::FromInt(rank) + FString(" ") + playerName.c_str() + FString(" ") + FString::SanitizeFloat(score));
				}
			}
		});
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::MakeRandomColor(), TEXT("Player is not conected"));
	}

}


void APlayerStateBase::FindMatch()
{

	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::FindMatch trying to find match with another player "))
	//GS Instance reference
	GameSparks::Core::GS& gs = UGameSparksModule::GetModulePtr()->GetGSInstance();

	// create request
	GameSparks::Api::Requests::MatchmakingRequest request(gs);

	//	{
	//		"@class": ".MatchmakingRequest",
	//			"matchShortCode" : "MATCH",
	//			"skill" : 0
	//	}

	request.SetMatchShortCode("MATCH");
	request.SetSkill(0);
	request.Send([&](GameSparks::Core::GS& gsI, const GameSparks::Api::Responses::MatchmakingResponse& response)
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::FindMatch CallBack! "))
		// TODO temp
		ShowChests();
	});
}

int32 APlayerStateBase::ParseChest(FString chestName)
{
	FString chestNumStr = chestName;
	chestNumStr.RemoveAt(0, 8, true);
	int32 chestNum = FCString::Atoi(*chestNumStr);
	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::ParseChest chest number %d "), chestNum)
	return chestNum;
}

int32 APlayerStateBase::ParsePlayer(FString chestName)
{
	FString chestNumStr = chestName.Replace(TEXT("Chest_"), TEXT(""), ESearchCase::IgnoreCase);
	FString playerNumStr = chestNumStr.Left(1);
	int32 playerNum = FCString::Atoi(*playerNumStr);
	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::ParsePlayer player number %d "), playerNum)
	return playerNum;
}

void APlayerStateBase::SendHideEvent(int chestNum)
{
	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendHideEvent (chest %d) send message to GS server "), chestNum)

	//GS Instance reference
	GameSparks::Core::GS& gs = UGameSparksModule::GetModulePtr()->GetGSInstance();

	//Create request instance
	GameSparks::Api::Requests::LogChallengeEventRequest hideRequest(gs);

	// set Challenge ID
	hideRequest.SetChallengeInstanceId(TCHAR_TO_UTF8(*challengeID));

	//Set Shortcode
	hideRequest.SetEventKey("Hide");

	int32 iX = chestNum / 2;
	int32 iY = chestNum % 2;

	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendHideEvent chest number %d ( X:%d , Y:%d )"), chestNum, iX, iY)

	hideRequest.SetEventAttribute("X", iX);
	hideRequest.SetEventAttribute("Y", iY);

	//Send request and reference response listener function
	hideRequest.Send([&](GameSparks::Core::GS&, const GameSparks::Api::Responses::LogChallengeEventResponse& response)
	{
		//Check if response is error free
		if (!response.GetHasErrors()) {
			UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendHideEvent callback : "))
				FString answer(response.GetJSONString().c_str());
			UE_LOG(LogTemp, Warning, TEXT("%s"), *answer)
		}
	});
}


void APlayerStateBase::SendInitPlayerEvent()
{
	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendInitPlayerEvent - send initialize message to GS server "))

	//GS Instance reference
	GameSparks::Core::GS& gs = UGameSparksModule::GetModulePtr()->GetGSInstance();

	//Create request instance
	GameSparks::Api::Requests::LogChallengeEventRequest initRequest(gs);

	// set Challenge ID
	initRequest.SetChallengeInstanceId(TCHAR_TO_UTF8(*challengeID));

	//Set Shortcode
	initRequest.SetEventKey("InitPlayer");

	//Send request and reference response listener function
	initRequest.Send([&](GameSparks::Core::GS&, const GameSparks::Api::Responses::LogChallengeEventResponse& response)
	{
		//Check if response is error free
		if (!response.GetHasErrors()) {
			UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendInitPlayerEvent callback : "))
			FString answer(response.GetJSONString().c_str());
			UE_LOG(LogTemp, Warning, TEXT("%s"), *answer)

				if (response.GetScriptData().GetValue().ContainsKey("chestPairsCount"))
				{

					roundState = EGameState::hideStage;
					UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendInitPlayerEvent callback : Begin Network Play"))
					// in chestPairsCount we recieve how many chest will be visible
					visiblePairs = response.GetScriptData().GetValue().GetInt("chestPairsCount").GetValue();
					UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendInitPlayerEvent callback : Starting HideStage, playing with %d pairs of chests."), visiblePairs)
					ShowChests();
					playerPoints = enemyPoints = visiblePairs;
					gameUI->ShowPlayerPoints(playerPoints);
					gameUI->ShowEnemyPoints(enemyPoints);
					gameUI->ShowRoundStatus("Hide");
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("APlayerStateBase::SendInitPlayerEvent callback : Starting  Begin Network Play error! We didnt recieve cout of visible chests-pair"))
				}

		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("APlayerStateBase::SendInitPlayerEvent - initialization error "))
		}
	});
}

void APlayerStateBase::SendFindEvent(int32 chestNum)
{
	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendFindEvent (chest %d) send message to GS server "), chestNum)

	//GS Instance reference
	GameSparks::Core::GS& gs = UGameSparksModule::GetModulePtr()->GetGSInstance();

	//Create request instance
	GameSparks::Api::Requests::LogChallengeEventRequest hideRequest(gs);

	// set Challenge ID
	hideRequest.SetChallengeInstanceId(TCHAR_TO_UTF8(*challengeID));

	//Set Shortcode
	hideRequest.SetEventKey("CheckChest");

	int32 iX = chestNum / 2;
	int32 iY = chestNum % 2;

	UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendFindEvent chest number %d ( X:%d , Y:%d )"), chestNum, iX, iY)

	hideRequest.SetEventAttribute("X", iX);
	hideRequest.SetEventAttribute("Y", iY);

	//Send request and reference response listener function
	hideRequest.Send([&](GameSparks::Core::GS&, const GameSparks::Api::Responses::LogChallengeEventResponse& response)
	{
		//Check if response is error free
		if (!response.GetHasErrors()) {
			UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendFindEvent callback : "))
			FString answer(response.GetJSONString().c_str());
			UE_LOG(LogTemp, Warning, TEXT("%s"), *answer)

			if (response.GetScriptData().GetValue().ContainsKey("Treasure"))
			{
				int32 sTreasure = response.GetScriptData().GetValue().GetInt("Treasure").GetValue();
				UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendFindEvent callback : treasure in chest %d"), sTreasure)
				if (sTreasure == 1)
				{
					int32 chestNumber = response.GetScriptData().GetValue().GetInt("Chest").GetValue();
					UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendFindEvent callback : chest number %d"), chestNumber)
					playerPoints++;
					enemyPoints--;
					gameUI->ShowPlayerPoints(playerPoints);
					gameUI->ShowEnemyPoints(enemyPoints);
					ShowTreasureInEnemyChest(chestNumber);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendFindEvent callback : cant find key 'Treasure' in script"))
			}
		}
	});
}

void APlayerStateBase::ChestClicked(FString chestName)
{
	int32 playerNum = ParsePlayer(chestName);
	int32 chestNum = ParseChest(chestName);
	int32 iPairNum = chestNum / 2;

	if ( isSinglePlayer || isUserAutorized)
	{
		// if we click on OWN chest in HIDE roundStage
		if ( roundState == EGameState::hideStage && ( (isPlayerOne && playerNum==1) || (!isPlayerOne && playerNum == 2) ))
		{
			UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::ChestClicked : player click on OWN chest to HIDE treasure "))
			
 			if (!clickCheck[iPairNum])
			{
				UE_LOG(LogTemp, Warning, TEXT("clickCheck[%d] = false "), iPairNum)
				OpenChestEvent(chestName, true);
				
				if (isSinglePlayer) 
				{
					playerChests[chestNum] = CHEST_WITH_TREASURE;
					playerTurns--;
					// if we hide all treasute , than log result of Player TArray
					if (playerTurns == 0)
					{
						LogOutTArray<int32>(enemyChests, "APlayerStateBase::AIchooseChest - PLAYER result chest array : ");
					}

				}
				else if (isUserAutorized)
				{
					
					SendHideEvent(chestNum);
				}
				clickCheck[iPairNum] = true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("clickCheck[%d] = iPairNum "), iPairNum)
			}
		}
		// if we click on ENEMY chest in FIND roundStage
		else if (roundState == EGameState::findStage && ((isPlayerOne && playerNum == 2) || (!isPlayerOne && playerNum == 1)))
		{
			if (!clickCheck[iPairNum])
			{
				UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::ChestClicked : player click on ENEMY chest to FIND treasure "))
				UE_LOG(LogTemp, Warning, TEXT("clickCheck[%d] = false "), iPairNum)
				if (isSinglePlayer)
				{
					// if player guess a treasure , then increase his points , and decrease AI points
					if (enemyChests[chestNum] == CHEST_WITH_TREASURE)
					{
						playerPoints++;
						enemyPoints--;
						gameUI->ShowPlayerPoints(playerPoints);
						gameUI->ShowEnemyPoints(enemyPoints);
						OpenChestEvent(chestName, true);
					}
					else
					{
						OpenChestEvent(chestName, false);
					}
					// decrease player turns
					playerTurns--;
				}
				else if (isUserAutorized)
				{
					SendFindEvent(chestNum);
					OpenChestEvent(chestName, false);
				}				
				clickCheck[iPairNum] = true;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("clickCheck[%d] = true "), iPairNum)
		}
		
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::MakeRandomColor(), TEXT("Player is not conected"));
	}

}


void APlayerStateBase::EnemyChestHideMessage_Implementation(int32 pairNum)
{
	FString debugMsg = "APlayerStateBase::EnemyChestHideMessage_Implementation \n Message from Enemy, it hide treasure in"; 
	debugMsg = debugMsg.Append(FString::FromInt(pairNum));
	debugMsg = debugMsg.Append(" chest");

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, debugMsg);
	UE_LOG(LogTemp, Warning, TEXT(""), *debugMsg)
}

void APlayerStateBase::OpenChestEvent_Implementation(const FString & chestName, bool isTreasure)
{

}

void APlayerStateBase::CloseChestEvent_Implementation(const FString & chestName)
{

}

void APlayerStateBase::recieveHideStageMessage_Implementation()
{

}

void APlayerStateBase::recieveFindStageMessage_Implementation()
{

}

void APlayerStateBase::ShowChests_Implementation()
{

}

void APlayerStateBase::StartSingleGame()
{
	SetDefaultValues();
	isPlayerOne = true;
	isSinglePlayer = true;
	AIturnsLeft = playerTurns = playerPoints = enemyPoints = singlePlayMaxTurns;

	newState = EGameState::hideStage;
	GetWorld()->GetTimerManager().SetTimer(changeRoundStateTimer, this, &APlayerStateBase::ChangeRoundStateSingle, roundStateDelay, false);
}

void APlayerStateBase::AIhideTreasure()
{
	if (AIturnsLeft > 0) 
	{
		int32 randValue = FMath::RandRange((int32)0, (int32)1);
		int32 chestIndex = (AIturnsLeft - 1) * 2 + randValue;
		UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::AIhideTreasure - AI turn ....., Hide in : %d"), randValue);
		FString chestName = "Chest_2_";
		chestName.Append(FString::FromInt(chestIndex));
		UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::AIhideTreasure - AI turn ....., Chest : %s"), *chestName);

		enemyChests[chestIndex] = CHEST_WITH_TREASURE;
		EnemyChestHideMessage(AIturnsLeft-1);
		AIturnsLeft--;
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(AImoveTimer);
		LogOutTArray<int32>(enemyChests, "APlayerStateBase::AIhideTreasure - AI result chest array : ");
	}
}

void APlayerStateBase::AIfindTreasure()
{
	if (AIturnsLeft > 0)
	{
		int32 randValue = FMath::RandRange((int32)0, (int32)1);
		int32 chestIndex = (AIturnsLeft - 1) * 2 + randValue;
		FString chestName = "Chest_1_";
		chestName.Append(FString::FromInt(chestIndex));
		UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::AIfindTreasure - AI try find in Chest : %s"), *chestName);

		if (playerChests[chestIndex] == CHEST_WITH_TREASURE)
		{
			OpenChestEvent(chestName, true);
			gameUI->ShowPlayerPoints(--playerPoints);
			gameUI->ShowEnemyPoints(++enemyPoints);
		}
		else
		{
			OpenChestEvent(chestName, false);
		}
		AIturnsLeft--;
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(AImoveTimer);
	}
}

void APlayerStateBase::ChangeRoundStateSingle()
{	
	GetWorld()->GetTimerManager().ClearTimer(changeRoundStateTimer);
	isTimerSet = false;

	switch (newState)
	{
	case EGameState::beginPlay:
		break;
	case EGameState::hideStage:
		CloseAllChest();
		// reset array, wich contains flags, did we click at the pair of chest or not ?
		clickCheck.Init(EMPTY_CHEST, playerTurns);
		AIturnsLeft = playerTurns = visiblePairs = FMath::Min(playerPoints, enemyPoints);
		ShowChests();
		roundState = EGameState::hideStage;
		gameUI->ShowRoundStatus("Hide");
		playerChests.Init(EMPTY_CHEST, AIturnsLeft * 2);
		enemyChests.Init(EMPTY_CHEST, playerTurns * 2);
		clickCheck.Init(EMPTY_CHEST, playerTurns);
		GetWorld()->GetTimerManager().SetTimer(AImoveTimer, this, &APlayerStateBase::AIhideTreasure, AIturnDelay, true, 0);
		break;
	case EGameState::findStage:
		CloseAllChest();
		AIturnsLeft = playerTurns = FMath::Min(playerPoints, enemyPoints);
		// reset array, wich contains flags, did we click at the pair of chest or not ?
		clickCheck.Init(EMPTY_CHEST, playerTurns);
		// log out array to UE_LOG
		LogOutTArray<bool>(clickCheck, "APlayerStateBase::ChangeRoundStateSingle : clickCheck ");
		// set new Type Of Round Stage ( when we finding treasure )
		roundState = EGameState::findStage;
		// show AT UI new stage
		gameUI->ShowRoundStatus("Find");
		GetWorld()->GetTimerManager().SetTimer(AImoveTimer, this, &APlayerStateBase::AIfindTreasure, AIturnDelay, true, 0);
		break;
	case EGameState::endPlay:
		roundState = EGameState::endPlay;
		break;
	default:
		break;
	}
}

void APlayerStateBase::SendGetFriendsList()
{
	if (isUserAutorized)
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendGetFriendsList - getting friends list GS server "))

		//GS Instance reference
		GameSparks::Core::GS& gs = UGameSparksModule::GetModulePtr()->GetGSInstance();

		//Create request instance
		GameSparks::Api::Requests::LogEventRequest request(gs);

		request.SetEventKey("getPlayerFriends");

		request.Send([&](GameSparks::Core::GS& gsI, const GameSparks::Api::Responses::LogEventResponse & response)
		{
			//check for errors
			if (response.GetHasErrors())
			{
				UE_LOG(LogTemp, Error, TEXT("APlayerStateBase::SendGetFriendsList callback : some errors when we get friends list !"))
			}
			else
			{

				FString responseStr = response.GetJSONString().c_str();
				UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendGetFriendsList callback : friends list %s"), *responseStr);

				auto varArr = response.GetScriptData().GetValue().GetGSDataObject("friendsList").GetValue();
				auto varKeys = varArr.GetKeys();

				PlayerFriendsList.Reset();

				for (auto varZnach : varKeys)
				{
					FriendListItem playerFriends;
					playerFriends.id = varZnach.c_str();
					UE_LOG(LogTemp, Warning, TEXT("list : %s"), *playerFriends.id);
					playerFriends.displayName = varArr.GetGSDataObject(varZnach).GetValue().GetString("displayName").GetValue().c_str();
					UE_LOG(LogTemp, Warning, TEXT("list : %s"), *playerFriends.displayName);
					PlayerFriendsList.Add(playerFriends);
				}

				//If data exists, then do something with it. GetBaseData function extracts the data from the response.
				if (response.GetBaseData().GetGSDataObject("scriptData").GetValue().GetFloat("friendsList").HasValue())
				{
					// response.GetBaseData().
					UE_LOG(LogTemp, Warning, TEXT("APlayerStateBase::SendGetFriendsList callback : friends list "));
						// SetPlayerScore(response.GetBaseData().GetGSDataObject("scriptData").GetValue().GetFloat("SCORE").GetValue());
				}
			}
		});
	}
}

