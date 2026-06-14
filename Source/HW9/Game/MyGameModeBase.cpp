// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MyGameModeBase.h"

#include "MyGameStateBase.h"
#include "Player/MyPlayerController.h"
#include "EngineUtils.h"
#include "Player/MyPlayerState.h"

void AMyGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	/**
	AMyGameStateBase* MyGameStateBase =  GetGameState<AMyGameStateBase>();
	if (IsValid(MyGameStateBase) == true)
	{
		MyGameStateBase->MulticastRPCBroadcastLoginMessage(TEXT("XXXXXXX"));
	}
	
	AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(NewPlayer);
	if (IsValid(MyPlayerController) == true)
	{
		AllPlayerControllers.Add(MyPlayerController);
	}
	**/
	
	AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(NewPlayer);
	if (IsValid(MyPlayerController) == true)
	{
		AllPlayerControllers.Add(MyPlayerController);
		
		MyPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));

		AMyPlayerState* MyPS = MyPlayerController->GetPlayerState<AMyPlayerState>();
		if (IsValid(MyPS) == true)
		{
			MyPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		AMyGameStateBase* MyGameStateBase =  GetGameState<AMyGameStateBase>();
		if (IsValid(MyGameStateBase) == true)
		{
			MyGameStateBase->MulticastRPCBroadcastLoginMessage(MyPS->PlayerNameString);
		}
		
		// 플레이어가 2명 이상일 시 게임 시작
		if (AllPlayerControllers.Num() >= 2)
		{
			CurrentPlayerIndex = 0;
			StartNewTurn();
		}
	}
}

FString AMyGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });
	
	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	SecretNumberString = GenerateSecretNumber();
}

bool AMyGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do {

		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}
			
			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
		{
			break;
		}

		bCanPlay = true;
		
	} while (false);	

	return bCanPlay;
}

FString AMyGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else 
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;				
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void AMyGameModeBase::PrintChatMessageString(AMyPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	FString GuessNumberString = InChatMessageString.TrimStartAndEnd();
	AMyPlayerState* MyPS = InChattingPlayerController->GetPlayerState<AMyPlayerState>();
	
	bool bIsGameStarted = (AllPlayerControllers.Num() >= 2);
	
	if (bIsGameStarted == true && IsGuessNumberString(GuessNumberString) == true)
	{
		if (AllPlayerControllers.IsValidIndex(CurrentPlayerIndex) == true)
		{
			if (AllPlayerControllers[CurrentPlayerIndex] != InChattingPlayerController)
			{
				// 내 차례가 아닌데 숫자를 입력했다면 리턴
				InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("It is not your turn yet!"));
				return;
			}
		}
	}
	
	bool bHasRemainingGuesses = false;
	if (IsValid(MyPS) == true)
	{
		bHasRemainingGuesses = MyPS->CurrentGuessCount < MyPS->MaxGuessCount;
	}

	// 야구 게임 입력인 경우
	if (bIsGameStarted == true && bHasRemainingGuesses == true && IsGuessNumberString(GuessNumberString) == true)
	{
		IncreaseGuessCount(InChattingPlayerController);
		
		FString PlayerInfoString = MyPS->GetPlayerInfoString();
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		
		// 최종 출력 문자열
		FString CombinedMessageString = PlayerInfoString + TEXT(": ") + GuessNumberString + TEXT(" : ") + JudgeResultString;
		
		for (TActorIterator<AMyPlayerController> It(GetWorld()); It; ++It)
		{
			AMyPlayerController* MyPlayerController = *It;
			if (IsValid(MyPlayerController) == true)
			{
				MyPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
			}
		}

		//판정 함수
		int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
		JudgeGame(InChattingPlayerController, StrikeCount);
		
		if (StrikeCount < 3)
		{
			SetNextTurn();
		}
	}
	
	//  일반 채팅이거나 횟수를 초과한 경우
	else
	{
		FString PlayerInfoString = IsValid(MyPS) ? MyPS->GetPlayerInfoString() : TEXT("Unknown");
		FString CombinedMessageString = PlayerInfoString + TEXT(": ") + InChatMessageString;

		for (TActorIterator<AMyPlayerController> It(GetWorld()); It; ++It)
		{
			AMyPlayerController* MyPlayerController = *It;
			if (IsValid(MyPlayerController) == true)
			{
				MyPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
			}
		}
	}
}

void AMyGameModeBase::IncreaseGuessCount(AMyPlayerController* InChattingPlayerController)
{
	AMyPlayerState* MyPS = InChattingPlayerController->GetPlayerState<AMyPlayerState>();
	if (IsValid(MyPS) == true)
	{
		MyPS->CurrentGuessCount++;
	}
}

void AMyGameModeBase::ResetGame()
{
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	
	float DelayTime = 3.0f; //3초 뒤 게임 재시작

	GetWorldTimerManager().SetTimer(
		ResetGameTimerHandle, 
		[this]()
		{
			SecretNumberString = GenerateSecretNumber();

			for (const auto& MyPlayerController : AllPlayerControllers)
			{
				if (IsValid(MyPlayerController) == true)
				{
					MyPlayerController->NotificationText = FText::FromString(TEXT("GameStart"));
					
					MyPlayerController->UpdateNotificationUI(); 
					
					AMyPlayerState* MyPS = MyPlayerController->GetPlayerState<AMyPlayerState>();
					if (IsValid(MyPS) == true)
					{
						MyPS->CurrentGuessCount = 0;
					}
				}
			}
			//턴 초기화
			CurrentPlayerIndex = 0;
			StartNewTurn();
		}, 
		DelayTime, 
		false 
	);
}

void AMyGameModeBase::JudgeGame(AMyPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		AMyPlayerState* MyPS = InChattingPlayerController->GetPlayerState<AMyPlayerState>();
		for (const auto& MyPlayerController : AllPlayerControllers)
		{
			if (IsValid(MyPS) == true)
			{
				FString CombinedMessageString = MyPS->PlayerNameString + TEXT(" has won the game.");
				MyPlayerController->NotificationText = FText::FromString(CombinedMessageString);
				ResetGame();
			}
		}
	}
	else
	{
		bool bIsDraw = true;
		for (const auto& MyPlayerController : AllPlayerControllers)
		{
			AMyPlayerState* MyPS = MyPlayerController->GetPlayerState<AMyPlayerState>();
			if (IsValid(MyPS) == true)
			{
				if (MyPS->CurrentGuessCount < MyPS->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}

		if (true == bIsDraw)
		{
			for (const auto& MyPlayerController : AllPlayerControllers)
			{
				MyPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));

				ResetGame();
			}
		}
	}
}

void AMyGameModeBase::StartNewTurn()
{
	if (AllPlayerControllers.IsValidIndex(CurrentPlayerIndex) == false) return;
	
	AMyPlayerController* CurrentPC = AllPlayerControllers[CurrentPlayerIndex];
	if (IsValid(CurrentPC) == true)
	{
		AMyPlayerState* MyPS = CurrentPC->GetPlayerState<AMyPlayerState>();
		FString TurnMessage = FString::Printf(TEXT("[%s]'s Turn! Please Input Number."), MyPS ? *MyPS->PlayerNameString : TEXT("Unknown"));
		for (const auto& PC : AllPlayerControllers)
		{
			if (IsValid(PC) == true)
			{
				PC->NotificationText = FText::FromString(TurnMessage);
				
				if (PC->IsLocalController() == true)
				{
					PC->UpdateNotificationUI();
				}
			}
		}
		
		StartTurnTimer(); // 타이머 세팅
	}
}

void AMyGameModeBase::SetNextTurn()
{
	if (AllPlayerControllers.Num() ==0) return;
	
	CurrentPlayerIndex = (CurrentPlayerIndex + 1) % AllPlayerControllers.Num();
	
	StartNewTurn();
}

void AMyGameModeBase::StartTurnTimer()
{
	AMyGameStateBase* MyGS = GetGameState<AMyGameStateBase>();
	if (IsValid(MyGS))
	{
		MyGS->RemainingTurnTime = MaxTurnTime;
	}
	
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	GetWorldTimerManager().SetTimer(TurnTimerHandle, this, &AMyGameModeBase::UpdateTurnTime, 1.0f, true);
}


void AMyGameModeBase::UpdateTurnTime()
{
	AMyGameStateBase* MyGS = GetGameState<AMyGameStateBase>();
	if (!IsValid(MyGS)) return;

	MyGS->RemainingTurnTime -= 1.0f;
	
	if (MyGS->RemainingTurnTime <= 0.0f)
	{
		GetWorldTimerManager().ClearTimer(TurnTimerHandle);
		OnTurnTimeOut();
	}
}

void AMyGameModeBase::OnTurnTimeOut() // 시간 초과
{
	AMyPlayerController* CurrentPC = nullptr;
	
	if (AllPlayerControllers.IsValidIndex(CurrentPlayerIndex) == true)
	{
		CurrentPC = AllPlayerControllers[CurrentPlayerIndex];
		
		if (IsValid(CurrentPC) == true)
		{
			IncreaseGuessCount(CurrentPC);
		}
	}
	
	for (const auto& PC : AllPlayerControllers)
	{
		if (IsValid(PC))
		{
			PC->ClientRPCPrintChatMessageString(TEXT("Time Out! Turn Switched."));
		}
	}
	if (IsValid(CurrentPC) == true)
	{
		JudgeGame(CurrentPC, 0); 
	}
	
	if (GetWorldTimerManager().IsTimerActive(ResetGameTimerHandle) == false)
	{
		SetNextTurn();
	}
}
