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
	
	UPROPERTY(ReplicatedUsing = OnRep_NotificationText, BlueprintReadOnly)
	FText NotificationText;
	
	UFUNCTION()
	void OnRep_NotificationText();
	
	void UpdateNotificationUI();
	
	void UpdateTurnTimerUI(float NewTime);
};
