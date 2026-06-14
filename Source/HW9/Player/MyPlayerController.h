// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UMyUserWidget;
class UNotificationWidget;
/**
 * 
 */
UCLASS()
class HW9_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	AMyPlayerController();
	
	virtual void BeginPlay() override;
	
	void SetChatMessageString(const FString& InChatMessageString);
	
	void PrintChatMessageString(const FString& InChatMessageString);
	
	
	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifeTimeProps) const override;
	
	
protected:
	// 채팅창 위젯
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMyUserWidget> ChatInputWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UMyUserWidget> ChatInputWidgetInstance;	
	
	// 경기 내용, 결과 출력 위젯
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNotificationWidget> NotificationTextWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UNotificationWidget> NotificationTextWidgetInstance;
	
	FString ChatMessageString;

public:
	// ⭐️ 수정: Replicated를 ReplicatedUsing으로 변경하여 Notify 함수를 지정합니다.
	UPROPERTY(ReplicatedUsing = OnRep_NotificationText, BlueprintReadOnly)
	FText NotificationText;

	// ⭐️ [새로 추가] 서버로부터 NotificationText를 복제받았을 때 UI를 동기화할 갱신 함수
	UFUNCTION()
	void OnRep_NotificationText();

	// ⭐️ [새로 추가] 현재 NotificationText 변수의 값을 위젯 텍스트 박스에 강제로 적용하는 헬퍼 함수
	void UpdateNotificationUI();
	
	void UpdateTurnTimerUI(float NewTime);
};
