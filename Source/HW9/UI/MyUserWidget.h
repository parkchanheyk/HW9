// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyUserWidget.generated.h"

class UEditableTextBox;
/**
 * 
 */
UCLASS()
class HW9_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget)) 
	TObjectPtr<UEditableTextBox> EditableTextBox_ChatInput;
	
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;
protected:
	UFUNCTION()
	void OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};
