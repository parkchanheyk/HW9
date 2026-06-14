// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

class AMyPlayerController;
/**
 * 
 */
UCLASS()
class HW9_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void OnPostLogin(AController* NewPlayer) override;	
	
	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);
	
public:	
	virtual void BeginPlay() override;
	
	void PrintChatMessageString(AMyPlayerController* InChattingPlayerController, const FString& InChatMessageString);
	
	void IncreaseGuessCount(AMyPlayerController* InChattingPlayerController);
	
	void ResetGame();

	void JudgeGame(AMyPlayerController* InChattingPlayerController, int InStrikeCount);
	
protected:
	FString SecretNumberString;

	TArray<TObjectPtr<AMyPlayerController>> AllPlayerControllers;
	
	FTimerHandle ResetGameTimerHandle;
	
	// 턴을 시작하고 관리하기 위해
	void StartNewTurn();
	void SetNextTurn();
	
	int32 CurrentPlayerIndex;
	
	// 타이머
	FTimerHandle TurnTimerHandle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	float MaxTurnTime = 30.0f; 

	void StartTurnTimer(); // 타이머 가동 시작 함수
	void UpdateTurnTime(); // 1초마다 실행되며 시간을 감소시킬 함수
	void OnTurnTimeOut();  // 0초가 되었을 때 타임아웃 처리 함수
};
