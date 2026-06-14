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
				// 내 차례가 아닌데 숫자를 입력했다면 당사자에게 경고
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

	// 1. 야구 게임 입력인 경우
	if (bIsGameStarted == true && bHasRemainingGuesses == true && IsGuessNumberString(GuessNumberString) == true)
	{
		// 먼저 횟수를 증가시킵니다. (이로써 0/3 이었던 상태가 1/3 로 정상 반영됨)
		IncreaseGuessCount(InChattingPlayerController);
		
		// 증가된 횟수가 반영된 최신 플레이어 정보를 가져와서 문자열을 조립합니다.
		FString PlayerInfoString = MyPS->GetPlayerInfoString();
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		
		// 최종 출력 문자열 (예: "Player1(1/3): 123 -> 1S0B")
		FString CombinedMessageString = PlayerInfoString + TEXT(": ") + GuessNumberString + TEXT(" : ") + JudgeResultString;
		
		for (TActorIterator<AMyPlayerController> It(GetWorld()); It; ++It)
		{
			AMyPlayerController* MyPlayerController = *It;
			if (IsValid(MyPlayerController) == true)
			{
				MyPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
			}
		}

		// (버그 수정) 판정 함수는 루프 밖에서 한 번만 호출되도록 수정했습니다.
		int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
		JudgeGame(InChattingPlayerController, StrikeCount);
		
		if (StrikeCount < 3)
		{
			SetNextTurn();
		}
	}
	// 2. 일반 채팅이거나 횟수를 초과한 경우
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
		false // 반복 여부 (단발성)
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
		// GameState의 복제 변수를 최대 시간(30초)으로 세팅합니다.
		MyGS->RemainingTurnTime = MaxTurnTime;
	}

	// 돌고 있던 타이머가 있다면 안전하게 끄고 새로 시작합니다.
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	// 1초마다 UpdateTurnTime 함수를 반복(true) 호출합니다.
	GetWorldTimerManager().SetTimer(TurnTimerHandle, this, &AMyGameModeBase::UpdateTurnTime, 1.0f, true);
}


void AMyGameModeBase::UpdateTurnTime()
{
	AMyGameStateBase* MyGS = GetGameState<AMyGameStateBase>();
	if (!IsValid(MyGS)) return;

	MyGS->RemainingTurnTime -= 1.0f;

	// 0초에 도달하면 타이머를 해제하고 타임아웃 액션을 취합니다.
	if (MyGS->RemainingTurnTime <= 0.0f)
	{
		GetWorldTimerManager().ClearTimer(TurnTimerHandle);
		OnTurnTimeOut();
	}
}

void AMyGameModeBase::OnTurnTimeOut()
{
	AMyPlayerController* CurrentPC = nullptr;
	
	// 1. 현재 턴을 진행 중이던 플레이어를 찾습니다.
	if (AllPlayerControllers.IsValidIndex(CurrentPlayerIndex) == true)
	{
		CurrentPC = AllPlayerControllers[CurrentPlayerIndex];
		
		if (IsValid(CurrentPC) == true)
		{
			// ⭐️ [핵심 해결]: 타임아웃된 플레이어의 기회 횟수를 차감(1 증가)시킵니다.
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

	// 3. 기회가 소진되었으므로, 모든 유저의 기회가 바닥나서 '무승부' 상황이 되었는지 판정합니다.
	if (IsValid(CurrentPC) == true)
	{
		// 타임아웃이므로 0스트라이크 상태로 판정 함수를 넘겨 무승부(Draw) 여부만 체크하게 합니다.
		JudgeGame(CurrentPC, 0); 
	}

	// 4. 만약 방금 무승부가 판정되어 게임 리셋(3초 딜레이)이 진행 중이 아니라면 다음 턴으로 넘깁니다.
	if (GetWorldTimerManager().IsTimerActive(ResetGameTimerHandle) == false)
	{
		SetNextTurn();
	}
}
