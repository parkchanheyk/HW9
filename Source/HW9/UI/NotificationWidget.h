// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NotificationWidget.generated.h"

/**
 * 
 */
UCLASS()
class HW9_API UNotificationWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ShowBaseballResult(const FString& ResultString);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ShowTurnMessage(const FString& TurnMessageString);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void RefreshTimerUI(float NewTime);
};
