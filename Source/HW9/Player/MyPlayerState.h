// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class HW9_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	
	AMyPlayerState();
	
	UPROPERTY(Replicated)
	FString PlayerNameString;
	
	UPROPERTY(Replicated)
	int32 CurrentGuessCount;

	UPROPERTY(Replicated)
	int32 MaxGuessCount;
	
	FString GetPlayerInfoString();
};
