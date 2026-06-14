// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MyPlayerController.h"

#include "AutomationTestExcludelist.h"
#include "UI/MyUserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HW9.h"
#include "EngineUtils.h"
#include "Game/MyGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceNull.h"
#include "Player/MyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "UI/NotificationWidget.h"
#include "Components/EditableTextBox.h"

AMyPlayerController::AMyPlayerController()
{
	bReplicates = true;
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

    if (IsLocalController() == false)
    {
    	return;
    }
	
	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);
	
	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UNotificationWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UMyUserWidget>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}
}

void AMyPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;
	
	if (IsLocalController() == true)
	{
		ServerRPCPrintChatMessageString(InChatMessageString);
	}
}
void AMyPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	//UKismetSystemLibrary::PrintString(this, ChatMessageString, true, true, FLinearColor::Red, 5.0f);
	
	//FString NetModeString = ChatXFunctionLibrary::GetNetModeString(this);
	//FString CombinedMessageString = FString::Printf(TEXT("%s: %s"), *NetModeString, *InChatMessageString);
	//ChatXFunctionLibrary::MyPrintString(this, CombinedMessageString, 10.f);
	
	ChatXFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);
}

void AMyPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
	
	if (IsValid(NotificationTextWidgetInstance) == true)
	{
		FString LeftSide, RightSide;
		if (InChatMessageString.Split(TEXT(" : "), &LeftSide, &RightSide, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			// RightSide에는 "1S0B", "OUT" 같은 순수 결과만 들어있으므로 이것만 BaseballResultText에 출력됩니다.
			NotificationTextWidgetInstance->ShowBaseballResult(RightSide);
		}
	}
}

void AMyPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	/**
	
	for (TActorIterator<AMyPlayerController> It(GetWorld()); It; ++It)
	{
		AMyPlayerController* CXPlayerController = *It;
		if (IsValid(CXPlayerController) == true)
		{
			CXPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
		}
	}
	
	**/
	
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		AMyGameModeBase* CXGM = Cast<AMyGameModeBase>(GM);
		if (IsValid(CXGM) == true)
		{
			CXGM->PrintChatMessageString(this, InChatMessageString);
			
		}
	}
}

void AMyPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}

void AMyPlayerController::OnRep_NotificationText()
{
	UpdateNotificationUI();
}

void AMyPlayerController::UpdateNotificationUI()
{
	if (IsLocalController() == false) return;

	if (IsValid(NotificationTextWidgetInstance) == true)
	{
		NotificationTextWidgetInstance->ShowTurnMessage(NotificationText.ToString());
	}
}

void AMyPlayerController::UpdateTurnTimerUI(float NewTime)
{
	if (IsLocalController() == false) return;
	
	if (IsValid(NotificationTextWidgetInstance) == true)
	{
		NotificationTextWidgetInstance->RefreshTimerUI(NewTime);
	}
	
	if (IsValid(ChatInputWidgetInstance) == true)
	{
		UEditableTextBox* ChatInputBox = ChatInputWidgetInstance->EditableTextBox_ChatInput;
		
		if (IsValid(ChatInputBox) == true)
		{
			// 시간이 만료되었을 때 잠금 처리
			if (NewTime <= 0.0f)
			{
				ChatInputBox->SetIsEnabled(false);
			}
			else
			{
				// 시간이 남아있으면 활성화
				if (ChatInputBox->GetIsEnabled() == false)
				{
					ChatInputBox->SetIsEnabled(true);
					
					// 활성화 직후 자동으로 커서(포커스) 복구
					GetWorld()->GetTimerManager().SetTimerForNextTick([this, ChatInputBox]()
					{
						if (IsValid(ChatInputBox) == true)
						{
							ChatInputBox->SetUserFocus(this);
						}
					});
				}
			}
		}
	}
}