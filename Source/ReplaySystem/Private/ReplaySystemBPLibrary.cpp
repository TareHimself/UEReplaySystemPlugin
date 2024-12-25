// Copyright Epic Games, Inc. All Rights Reserved.

#include "ReplaySystemBPLibrary.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/DemoNetDriver.h"
#include "NetworkReplayStreaming.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "ReplayPlayerController.h"
#include "Containers/UnrealString.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Pawn.h"
#include "Serialization/MemoryReader.h"


UReplaySystemBPLibrary::UReplaySystemBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UReplaySystemBPLibrary::SetReplaySavePath(const FString& Path)
{
	if (const TSharedPtr<INetworkReplayStreamer> EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().
			CreateReplayStreamer())
	{
		EnumerateStreamsPtr->SetDemoPath(Path);
	}
}

FString UReplaySystemBPLibrary::GetReplaySavePath()
{
	FString Path = "";
	if (const TSharedPtr<INetworkReplayStreamer> EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().
			CreateReplayStreamer())
	{
		EnumerateStreamsPtr->GetDemoPath(Path);
	}
	return Path;
}

void UReplaySystemBPLibrary::RecordReplay(UObject* WorldContextObject, const FString& ReplayName,
                                          const FString& ReplayFriendlyName)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (UGameInstance* GI = Cast<UGameInstance>(World->GetGameInstance()))
		{
			const TArray<FString> Options;

			GI->StartRecordingReplay(ReplayName, ReplayFriendlyName, Options);
		}
	}
}

void UReplaySystemBPLibrary::StopRecordingReplay(UObject* WorldContextObject)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (IsRecordingReplay(WorldContextObject))
		{
			if (UGameInstance* GI = Cast<UGameInstance>(World->GetGameInstance()))
			{
				GI->StopRecordingReplay();
			}
		}
	}
}

bool UReplaySystemBPLibrary::IsRecordingReplay(UObject* WorldContextObject)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (World != nullptr && GetDemoDriver(World) != nullptr)
		{
			return GetDemoDriver(World)->IsRecording();
		}
	}
	return false;
}

void UReplaySystemBPLibrary::DeleteReplay(const FString& ReplayName,
                                          FOnDeleteReplayComplete OnDeleteComplete)
{
	const auto EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().CreateReplayStreamer();

	const auto OnDeleteCompleteDel = FDeleteFinishedStreamCallback::CreateLambda(
		[OnDeleteComplete](const FDeleteFinishedStreamResult& Result)
		{
			OnDeleteComplete.Execute(Result.WasSuccessful());
		});

	if (EnumerateStreamsPtr.Get())
	{
		EnumerateStreamsPtr.Get()->DeleteFinishedStream(ReplayName, OnDeleteCompleteDel);
	}
}

void UReplaySystemBPLibrary::RenameReplay(const FString& ReplayName,
                                          const FString& NewReplayName, const int32 UserIndex,
                                          FOnRenameReplayComplete OnRenameComplete)
{
	const auto EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().CreateReplayStreamer();

	const auto Delegate = FRenameReplayCallback::CreateLambda(
		[OnRenameComplete](const FRenameReplayResult& Result)
		{
			OnRenameComplete.Execute(Result.WasSuccessful());
		});

	if (EnumerateStreamsPtr.Get())
	{
		EnumerateStreamsPtr.Get()->RenameReplay(ReplayName, NewReplayName, UserIndex, Delegate);
	}
}

void UReplaySystemBPLibrary::RenameReplayFriendly(
                                                  const FString& ReplayName,
                                                  const FString& NewFriendlyReplayName,
                                                  const int32 UserIndex, FOnRenameReplayComplete OnRenameComplete)
{
	const auto EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().CreateReplayStreamer();

	const auto Delegate = FRenameReplayCallback::CreateLambda([OnRenameComplete](const FRenameReplayResult& Result)
	{
		OnRenameComplete.Execute(Result.WasSuccessful());
	});

	if (EnumerateStreamsPtr.Get())
	{
		EnumerateStreamsPtr.Get()->RenameReplayFriendlyName(ReplayName, NewFriendlyReplayName, UserIndex, Delegate);
	}
}

void UReplaySystemBPLibrary::GetSavedReplays(FOnGetReplaysComplete OnGetReplaysComplete)
{
	const auto EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().CreateReplayStreamer();

	const auto Delegate = FEnumerateStreamsCallback::CreateLambda(
		[OnGetReplaysComplete](const FEnumerateStreamsResult& Result)
		{
			TArray<FReplayInfo> Replays;

			TArray<FNetworkReplayStreamInfo> StreamInfos = Result.FoundStreams;

			for (FNetworkReplayStreamInfo StreamInfo : StreamInfos)
			{
				FReplayInfo ReplayInfo;
				ReplayInfo.FriendlyName = StreamInfo.FriendlyName;
				ReplayInfo.ActualName = StreamInfo.Name;
				ReplayInfo.RecordDate = StreamInfo.Timestamp;
				ReplayInfo.LengthInMS = StreamInfo.LengthInMS;
				const float SizeInKb = StreamInfo.SizeInBytes / 1024.0f;
				ReplayInfo.SizeInMb = SizeInKb / 1024.0f;
				
				Replays.Add(ReplayInfo);
			}

			OnGetReplaysComplete.Execute(Replays);
		});

	if (EnumerateStreamsPtr.Get())
	{
		EnumerateStreamsPtr.Get()->EnumerateStreams(FNetworkReplayVersion(), INDEX_NONE, FString(),
													TArray<FString>(), Delegate);
	}
}

bool UReplaySystemBPLibrary::PlayRecordedReplay(UObject* WorldContextObject, const FString& ReplayName)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (UGameInstance* GI = Cast<UGameInstance>(World->GetGameInstance()))
		{
			const TArray<FString> Options;

			return GI->PlayReplay(ReplayName, nullptr, Options);
		}
	}

	return false;
}

void UReplaySystemBPLibrary::RestartReplayPlayback(UObject* WorldContextObject, FOnGotoTimeComplete OnComplete)
{
	GoToSpecificTime(WorldContextObject,0.0f,false,OnComplete);
}

void UReplaySystemBPLibrary::GoToSpecificTime(UObject* WorldContextObject, float TimeToGoTo,
                                              bool bRetainCurrentPauseState, FOnGotoTimeComplete OnComplete)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (const AWorldSettings* WorldSettings = World->GetWorldSettings())
		{
			// clamp time just in case
			const float ClampedTime = UKismetMathLibrary::FClamp(TimeToGoTo, 0.0f,GetReplayLength(
				                                                     WorldContextObject));

			if (UDemoNetDriver* DemoDriver = GetDemoDriver(World))
			{
				if (DemoDriver->ServerConnection->PlayerController->PlayerState)
				{
					bool bPauseStateBeforeMove = false;
					
					if (const AReplayPlayerController* ReplayPC = Cast<AReplayPlayerController>(
						DemoDriver->ServerConnection->PlayerController))
					{
						if (ReplayPC->bIsSpectating)
						{
							if (const AActor* ActorBeingSpectated = Cast<AActor>(ReplayPC->GetViewTarget()))
							{
								const FNetworkGUID ActorBeingSpectatedGUID = DemoDriver->GetGUIDForActor(
									ActorBeingSpectated);
								DemoDriver->AddNonQueuedGUIDForScrubbing(ActorBeingSpectatedGUID);
							}
						}

						if (const AActor* ActorPossessed = Cast<AActor>(ReplayPC->GetPawn()))
						{
							const FNetworkGUID SpectatorPawnGUID = DemoDriver->GetGUIDForActor(ActorPossessed);
							DemoDriver->AddNonQueuedGUIDForScrubbing(SpectatorPawnGUID);
						}
					}
					else
					{
						if (const APlayerController* PC = Cast<APlayerController>(
							DemoDriver->ServerConnection->PlayerController))
						{
							if (const AActor* ActorPossessed = Cast<AActor>(PC->GetPawn()))
							{
								const FNetworkGUID SpectatorPawnGUID = DemoDriver->GetGUIDForActor(ActorPossessed);
								DemoDriver->AddNonQueuedGUIDForScrubbing(SpectatorPawnGUID);
							}
						}
					}

					for (FActorIterator It(World->GetWorld()); It; ++It)
					{
						if (It->bAlwaysRelevant)
						{
							const FNetworkGUID ActorGUID = DemoDriver->GetGUIDForActor(*It);
							DemoDriver->AddNonQueuedGUIDForScrubbing(ActorGUID);
						}
					}


					if (WorldSettings->GetPauserPlayerState())
					{
						bPauseStateBeforeMove = true;
					}

					const auto OnGoToTimeDelegate = FOnGotoTimeDelegate::CreateLambda(
						[OnComplete,bRetainCurrentPauseState,bPauseStateBeforeMove,World,WorldContextObject](bool bWasSuccessful)
						{
							OnComplete.Execute(bWasSuccessful);

							if (bRetainCurrentPauseState && !bPauseStateBeforeMove == false)
							{
								PausePlayback(WorldContextObject);
							}

							if (AReplayPlayerController* ReplayPC = Cast<AReplayPlayerController>(
								UGameplayStatics::GetPlayerController(World, 0)))
							{
								ReplayPC->OnGoToTime(GetCurrentReplayTime(WorldContextObject));
								ReplayPC->OnStopSpectateActor();
							}
						});

					DemoDriver->GotoTimeInSeconds(ClampedTime, OnGoToTimeDelegate);
				}
			}
		}
	}
}

void UReplaySystemBPLibrary::PausePlayback(UObject* WorldContextObject)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (AWorldSettings* WorldSettings = World->GetWorldSettings())
		{
			if (const UDemoNetDriver* DemoDriver = GetDemoDriver(World))
			{
				if (DemoDriver->ServerConnection->PlayerController->PlayerState)
				{
					World->bIsCameraMoveableWhenPaused = true;
					WorldSettings->SetPauserPlayerState(DemoDriver->ServerConnection->PlayerController->PlayerState);
					if (AReplayPlayerController* ReplayPC = Cast<AReplayPlayerController>(
						UGameplayStatics::GetPlayerController(World, 0)))
					{
						ReplayPC->OnTogglePause(true);
					}
				}
			}
		}
	}
}

void UReplaySystemBPLibrary::ResumePlayback(UObject* WorldContextObject)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (AWorldSettings* WorldSettings = World->GetWorldSettings())
		{
			WorldSettings->SetPauserPlayerState(nullptr);
			if (AReplayPlayerController* ReplayPC = Cast<AReplayPlayerController>(
				UGameplayStatics::GetPlayerController(World, 0)))
			{
				ReplayPC->OnTogglePause(false);
			}
		}
	}
}

void UReplaySystemBPLibrary::SetPlaybackSpeed(UObject* WorldContextObject, float Speed)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (AWorldSettings* WorldSettings = World->GetWorldSettings())
		{
			WorldSettings->DemoPlayTimeDilation = Speed;
		}
	}
}

float UReplaySystemBPLibrary::GetPlaybackSpeed(UObject* WorldContextObject)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (const AWorldSettings* WorldSettings = World->GetWorldSettings())
		{
			return WorldSettings->DemoPlayTimeDilation;
		}
	}
	return 1.0f;
}

float UReplaySystemBPLibrary::GetCurrentReplayTime(UObject* WorldContextObject)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (const UDemoNetDriver* DemoDriver = GetDemoDriver(World))
		{
			return DemoDriver->GetDemoCurrentTime();
		}
		else
		{
			return 0.0f;
		}
	}
	return 0.0f;
}

float UReplaySystemBPLibrary::GetReplayLength(UObject* WorldContextObject)
{
	if (auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (const UDemoNetDriver* DemoDriver = GetDemoDriver(World))
		{
			if (IsRecordingReplay(WorldContextObject))
			{
				return DemoDriver->AccumulatedRecordTime;
			}

			if (IsPlayingReplay(WorldContextObject))
			{
				return DemoDriver->GetDemoTotalTime();
			}
		}
	}
	return 0.0f;
}

bool UReplaySystemBPLibrary::IsPlayingReplay(UObject* WorldContextObject)
{
	if (auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (const UDemoNetDriver* DemoDriver = GetDemoDriver(World))
		{
			return DemoDriver->IsPlaying();
		}
	}
	return false;
}

bool UReplaySystemBPLibrary::IsReplayPlaybackPaused(UObject* WorldContextObject)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (IsPlayingReplay(WorldContextObject))
		{
			if (const AWorldSettings* WorldSettings = World->GetWorldSettings())
			{
				return WorldSettings->GetPauserPlayerState() != nullptr;
			}
		}
	}
	return false;
}

FString UReplaySystemBPLibrary::GetActiveReplayName(UObject* WorldContextObject)
{
	if (auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (World != nullptr && GetDemoDriver(World) != nullptr)
		{
			return GetDemoDriver(World)->GetActiveReplayName();
		}
	}
	return "None";
}

bool UReplaySystemBPLibrary::AddEventToActiveReplay(UObject* WorldContextObject, const FString& EventId,
                                                    const FString& Group, FString Metadata, TArray<uint8> Data)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (IsRecordingReplay(WorldContextObject))
		{
			if (World != nullptr && GetDemoDriver(World) != nullptr)
			{
				GetDemoDriver(World)->AddOrUpdateEvent(EventId, Group, Metadata, Data);

				return true;
			}
		}
	}
	return false;
}

void UReplaySystemBPLibrary::GetActiveReplayEvents(UObject* WorldContextObject, FString Group,
                                                       int UserIndex, FOnRequestEventsComplete OnRequestEventsComplete)
{
	if (!IsRecordingReplay(WorldContextObject) && IsPlayingReplay(WorldContextObject))
	{
		const TSharedPtr<INetworkReplayStreamer> EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().
			CreateReplayStreamer();


		const auto EnumerateEventsDel = FEnumerateEventsCallback::CreateLambda(
			[OnRequestEventsComplete](const FEnumerateEventsResult& Results)
			{
				TArray<FReplayEvent> ReplayEvents;
				if (Results.WasSuccessful())
				{
					for (FReplayEventListItem EventItem : Results.ReplayEventList.ReplayEvents)
					{
						FReplayEvent ReplayEventTemp;
						ReplayEventTemp.EventID = EventItem.ID;

						ReplayEventTemp.Group = EventItem.Group;
						ReplayEventTemp.TimeInMs = static_cast<int32>(EventItem.Time1);
						ReplayEventTemp.Metadata = EventItem.Metadata;

						ReplayEvents.Add(ReplayEventTemp);
					}
				}
				OnRequestEventsComplete.Execute(ReplayEvents);
			});

		if (EnumerateStreamsPtr.Get())
		{
			EnumerateStreamsPtr.Get()->EnumerateEvents(GetActiveReplayName(WorldContextObject), Group, UserIndex,
													   EnumerateEventsDel);
		}
	}
}

void UReplaySystemBPLibrary::GetDataForEvent(FString ReplayActualName, FString EventId,
                                             int UserIndex, FOnGetEventDataComplete OnGetEventDataComplete)
{
	const TSharedPtr<INetworkReplayStreamer> EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().
			CreateReplayStreamer();


	const auto RequestEventsDataDel = FRequestEventDataCallback::CreateLambda(
		[OnGetEventDataComplete](const FRequestEventDataResult& Result)
		{
			TArray<uint8> Data;
			if (Result.WasSuccessful())
			{
				Data = Result.ReplayEventListItem;
			}
			OnGetEventDataComplete.Execute(Data);
		});

	if (EnumerateStreamsPtr.Get())
	{
		EnumerateStreamsPtr.Get()->RequestEventData(ReplayActualName, EventId, UserIndex, RequestEventsDataDel);
	}
}

void UReplaySystemBPLibrary::GetEvents(FString ReplayActualName, FString Group, int UserIndex,
	FOnRequestEventsComplete OnRequestEventsComplete)
{
	const TSharedPtr<INetworkReplayStreamer> EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().
		CreateReplayStreamer();

	const auto EnumerateEventsDel = FEnumerateEventsCallback::CreateLambda(
		[OnRequestEventsComplete](const FEnumerateEventsResult& Results)
		{
			TArray<FReplayEvent> ReplayEvents;
			if (Results.WasSuccessful())
			{
				for (FReplayEventListItem EventItem : Results.ReplayEventList.ReplayEvents)
				{
					FReplayEvent ReplayEventTemp;
					ReplayEventTemp.EventID = EventItem.ID;

					ReplayEventTemp.Group = EventItem.Group;
					ReplayEventTemp.TimeInMs = static_cast<int32>(EventItem.Time1);
					ReplayEventTemp.Metadata = EventItem.Metadata;
					ReplayEvents.Add(ReplayEventTemp);
				}
			}

			OnRequestEventsComplete.Execute(ReplayEvents);
		});

	if (EnumerateStreamsPtr.Get())
	{
		EnumerateStreamsPtr.Get()->EnumerateEvents(ReplayActualName, Group, UserIndex, EnumerateEventsDel);
	}
}

float UReplaySystemBPLibrary::MsToSeconds(const int32 MS)
{
#if  ENGINE_MAJOR_VERSION <= 4
	const float MsAsFloat = UKismetMathLibrary::Conv_IntToFloat(MS);
#else
	const float MsAsFloat = UKismetMathLibrary::Conv_IntToDouble(MS);
#endif

	const float MsAsSeconds = MsAsFloat / 1000;
	return MsAsSeconds;
}

void UReplaySystemBPLibrary::SpectateActor(UObject* WorldContextObject, AActor* Actor, FBlendSettings BlendSettings)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (!Actor)
		{
			return;
		}

		if (UGameplayStatics::GetPlayerController(World, 0))
		{
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
			{
				if (!PC->IsLocalController())
				{
					return;
				}

				//Everything from here is happening only locally/on the client
				PC->SetViewTargetWithBlend(Actor, BlendSettings.BlendTime, BlendSettings.BlendFunction,
				                           BlendSettings.BlendExponent, BlendSettings.bLockOutgoing);
				if (AReplayPlayerController* ReplayPC = Cast<AReplayPlayerController>(
					UGameplayStatics::GetPlayerController(World, 0)))
				{
					ReplayPC->OnSpectateActor(Actor);
					ReplayPC->bIsSpectating = true;
				}
			}
		}
	}
}

void UReplaySystemBPLibrary::StopSpectating(UObject* WorldContextObject, FBlendSettings BlendSettings)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (UGameplayStatics::GetPlayerController(World, 0))
		{
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
			{
				if (!PC->IsLocalController())
				{
					return;
				}

				//Everything from here is happening only locally/on the client

				auto Target = PC->GetPawn();
				PC->SetViewTargetWithBlend(Target, BlendSettings.BlendTime, BlendSettings.BlendFunction,
				                           BlendSettings.BlendExponent, BlendSettings.bLockOutgoing);

				if (AReplayPlayerController* ReplayPC = Cast<AReplayPlayerController>(
					UGameplayStatics::GetPlayerController(World, 0)))
				{
					ReplayPC->OnStopSpectateActor();
					ReplayPC->bIsSpectating = false;
				}
			}
		}
	}
}

UDemoNetDriver* UReplaySystemBPLibrary::GetDemoDriver(const UObject* WorldContextObject)
{
	if (auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
#if  ENGINE_MAJOR_VERSION > 4
		return World->GetDemoNetDriver();
#elif ENGINE_MINOR_VERSION >= 26
		return World->GetDemoNetDriver();
#elif ENGINE_MINOR_VERSION < 26
		return World->DemoNetDriver;
#endif
	}

	return nullptr;
}

void UReplaySystemBPLibrary::SetMaxRecordHz(UObject* WorldContextObject, float Hz)
{
	if (const auto World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (UGameplayStatics::GetPlayerController(World, 0))
		{
#if  ENGINE_MAJOR_VERSION <= 4
			FString Command = "demo.recordhz " + UKismetStringLibrary::Conv_FloatToString(Hz);
#else
			FString Command = "demo.recordhz " + FString::SanitizeFloat(Hz);
#endif


			UGameplayStatics::GetPlayerController(World, 0)->ConsoleCommand(Command);
		}
	}
}


float UReplaySystemBPLibrary::GetMaxRecordHz()
{
	static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("demo.recordhz"));
	const float value = CVar->GetFloat();
	return value;
}

void UReplaySystemBPLibrary::SerializeStruct(UStruct* Struct, TArray<uint8>& Data)
{
}

void UReplaySystemBPLibrary::DeSerializeStruct(TArray<uint8> Data, UStruct*& Struct)
{
}

TArray<uint8> UReplaySystemBPLibrary::StringToBytes(FString Data)
{
	TArray<uint8> Result;
	FMemoryWriter Ar(Result);
	Ar << Data;
	return Result;
}

FString UReplaySystemBPLibrary::BytesToString(TArray<uint8> Data)
{
	FString Result;
	FMemoryReader Ar(Data);
	Ar << Result;
	return Result;
}
