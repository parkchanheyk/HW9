// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MyGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "Player/MyPlayerController.h"
#include "Net/UnrealNetwork.h"

void AMyGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			AMyPlayerController* CXPC = Cast<AMyPlayerController>(PC);
			if (IsValid(CXPC) == true)
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				CXPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}

AMyGameStateBase::AMyGameStateBase()
{
	bReplicates = true; // 레플리케이션 옵션 활성화
}

void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMyGameStateBase, RemainingTurnTime); // 복제할 변수 추가
}

void AMyGameStateBase::OnRep_RemainingTurnTime()
{
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (IsValid(LocalPC))
	{
		AMyPlayerController* MyPC = Cast<AMyPlayerController>(LocalPC);
		if (IsValid(MyPC))
		{
			// PlayerController에 새로 만들 타이머 UI 업데이트 함수를 호출합니다.
			MyPC->UpdateTurnTimerUI(RemainingTurnTime);
		}
	}
}
