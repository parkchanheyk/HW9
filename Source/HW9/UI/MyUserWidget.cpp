// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MyUserWidget.h"
#include "Components/EditableTextBox.h"
#include "Player/MyPlayerController.h"

void UMyUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextCommitted);		
	}	
}

void UMyUserWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UMyUserWidget::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		APlayerController* OwningPlayerController = GetOwningPlayer();
		if (IsValid(OwningPlayerController) == true)
		{
			AMyPlayerController* OwningCXPlayerController = Cast<AMyPlayerController>(OwningPlayerController);
			if (IsValid(OwningCXPlayerController) == true)
			{
				OwningCXPlayerController->SetChatMessageString(Text.ToString());

				EditableTextBox_ChatInput->SetText(FText());
				
				// 텍스트 입력 창에 계속 포커싱 되게
				GetWorld()->GetTimerManager().SetTimerForNextTick([this, OwningCXPlayerController]()
				{
					if (IsValid(EditableTextBox_ChatInput))
					{
						EditableTextBox_ChatInput->SetUserFocus(OwningCXPlayerController);
					}
				});
			}
		}
	}
}
