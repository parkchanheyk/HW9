// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class HW9_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
public:
	AMyGameStateBase();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCBroadcastLoginMessage(const FString& InNameString = FString(TEXT("XXXXXXX")));
	
	// 네트워크 복제를 위한 함수
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	//시간 변수
	UPROPERTY(ReplicatedUsing = OnRep_RemainingTurnTime, BlueprintReadOnly, Category="GameState")
	float RemainingTurnTime = 0.0f;
	// 레플리케이션이 일어날 때 마다 호출할 함수
	UFUNCTION()
	void OnRep_RemainingTurnTime();
};
