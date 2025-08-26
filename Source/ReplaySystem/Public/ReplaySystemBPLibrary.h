// Copyright 2020-Present Oyintare Ebelo. All Rights Reserved.

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "ReplayDelegates.h"
#include "ReplayStructs.h"
#include "ReplaySystemBPLibrary.generated.h"


/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

class UInstantReplayObject;

UCLASS()
class UReplaySystemBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	/**
	 *  Sets the path for all replays (Does not copy existing replays over)
	 * @param Path New path
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void SetReplaySavePath(const FString& Path);

	/**
	 *  Gets the current path where replays are stored
	 * @return Path
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static FString GetReplaySavePath();


	/**
	 *  Starts recording a replay
	 * @param WorldContextObject 
	 * @param ReplayName The name to save the replay as on disk (Used internally)
	 * @param ReplayFriendlyName The Ui friendly name of the replay (Used by you or users)
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem|Recording",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void RecordReplay(UObject* WorldContextObject, const FString& ReplayName,
	                         const FString& ReplayFriendlyName);

	/**
	 * Stop recording a replay
	 * @param WorldContextObject 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem|Recording",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void StopRecordingReplay(UObject* WorldContextObject);


	/**
	 *  Finds out if a replay is being recorded
	 * @param WorldContextObject 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static bool IsRecordingReplay(UObject* WorldContextObject);

	/**
	 *  Delete a replay
	 * @param ReplayName The name the replay is saved as on disk
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void DeleteReplay( const FString& ReplayName, FOnDeleteReplayComplete OnDeleteComplete);

	/**
	 *  Changes the name a replay is saved as on the disk
	 * @param ReplayName The current name on disk of this replay
	 * @param NewReplayName New name to save as on disk
	 * @param UserIndex
	 * @param OnRenameComplete 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void RenameReplay(const FString& ReplayName,
	                                         const FString& NewReplayName, const int32 UserIndex,FOnRenameReplayComplete OnRenameComplete);

	/**
	 *  Changes the friendly name of a replay
	 * @param ReplayName The current name on disk of this replay
	 * @param NewFriendlyReplayName New friendly to give it
	 * @param UserIndex
	 * @param OnRenameComplete 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void RenameReplayFriendly(const FString& ReplayName,
	                                                 const FString& NewFriendlyReplayName, const int32 UserIndex,FOnRenameReplayComplete OnRenameComplete);

	/**
	 *  Get all the saved replays
	 * @param WorldContextObject
	 * @param OnGetReplaysComplete 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void GetSavedReplays(FOnGetReplaysComplete OnGetReplaysComplete);

	/**
	 *  Play a recorded replay
	 * @param WorldContextObject 
	 * @param ReplayName
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem|Playback",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static bool PlayRecordedReplay(UObject* WorldContextObject, const FString& ReplayName);

	/**
	 *  Restart the currently playing replay
	 * @param WorldContextObject
	 * @param OnComplete 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem|Playback",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void RestartReplayPlayback(UObject* WorldContextObject,FOnGotoTimeComplete OnComplete);

	/**
	 *  Goto a specific time in the replay
	 * @param WorldContextObject 
	 * @param TimeToGoTo Time in seconds to goto
	 * @param bRetainCurrentPauseState Use this on a need basis as it can cause some physics issues
	 * @param OnComplete
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem|Playback",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void GoToSpecificTime(UObject* WorldContextObject, float TimeToGoTo,
	                                         bool bRetainCurrentPauseState, FOnGotoTimeComplete OnComplete);


	/**
	 *  Pause the replay playback
	 * @param WorldContextObject 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem|Playback",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void PausePlayback(UObject* WorldContextObject);

	/**
	 *  Resume the replay playback
	 * @param WorldContextObject 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem|Playback",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void ResumePlayback(UObject* WorldContextObject);

	/**
	 *  Sets the Playback speed of the replay
	 * @param WorldContextObject Set the playback speed of the replay
	 * @param Speed Like time dilation 0.1,1,10
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem|Playback",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void SetPlaybackSpeed(UObject* WorldContextObject, float Speed);


	/**
	 *  Get the playback speed of the replay
	 * @param WorldContextObject 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ReplaySystem|Playback",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static float GetPlaybackSpeed(UObject* WorldContextObject);


	/**
	 *  Get the current time in seconds of the replay
	 * @param WorldContextObject 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ReplaySystem|Playback",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static float GetCurrentReplayTime(UObject* WorldContextObject);


	/**
	 *  Get the total length in seconds of the replay currently playing or being recorded
	 * @param WorldContextObject 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ReplaySystem|Playback",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static float GetReplayLength(UObject* WorldContextObject);


	/**
	 *  returns true if a replay is being played
	 * @param WorldContextObject 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static bool IsPlayingReplay(UObject* WorldContextObject);

	/**
	 * Returns true of the replay has been paused
	 * @param WorldContextObject 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static bool IsReplayPlaybackPaused(UObject* WorldContextObject);

	/**
	 *  Gets the name on disk/memory of the replay currently playing or being recorded
	 * @param WorldContextObject 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static FString GetActiveReplayName(UObject* WorldContextObject);


	/**
	 * Adds or Updates said event in the replay currently being recorded
	 * @param WorldContextObject 
	 * @param EventId The id of the event
	 * @param Group The group this event belongs to 
	 * @param Metadata Metadata As A String
	 * @param Data Data To Store for this event
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static bool AddEventToActiveReplay(UObject* WorldContextObject, const FString& EventId, const FString& Group,
	                                   FString Metadata, TArray<uint8> Data);

	/**
	 *  Gets the Events of the replay currently playing
	 * @param WorldContextObject 
	 * @param Group The group name
	 * @param UserIndex
	 * @param OnRequestEventsComplete Callback
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	void GetActiveReplayEvents(UObject* WorldContextObject, FString Group, int UserIndex,FOnRequestEventsComplete OnRequestEventsComplete);


	/**
	 * @brief 
	 * @param ReplayActualName The Actual Name Of The Replay
	 * @param EventId 
	 * @param UserIndex 
	 * @param OnGetEventDataComplete 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void GetDataForEvent(FString ReplayActualName, FString EventId,
	int UserIndex, FOnGetEventDataComplete OnGetEventDataComplete);

	/**
	 *  Gets the events of a replay
	 * @param ReplayActualName The Actual Name Of The Replay
	 * @param Group 
	 * @param UserIndex
	 * @param OnRequestEventsComplete 
	 */
	UFUNCTION(BlueprintCallable, Category = "ReplaySystem")
	static void GetEvents(FString ReplayActualName,FString Group,int UserIndex,FOnRequestEventsComplete OnRequestEventsComplete);
	
	/**
	 *  Helper function to convert milliseconds to seconds
	 * @param MS 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ReplaySystem|Utilities")
	static float MsToSeconds(const int32 MS);

	/**
	 *  Just sets the view target of the controller to the specified actor
	 * @param WorldContextObject
	 * @param Actor The pawn to spectate
	 */
	UFUNCTION(BlueprintCallable, Category= "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void SpectateActor(UObject* WorldContextObject, AActor* Actor, FBlendSettings BlendSettings);

	/**
	 *  Just sets the view target of the controller back to the possessed pawn
	 * @param WorldContextObject 
	 * @param BlendSettings 
	 */
	UFUNCTION(BlueprintCallable, Category= "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void StopSpectating(UObject* WorldContextObject, FBlendSettings BlendSettings);

	UFUNCTION(Category= "ReplaySystem", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static UDemoNetDriver* GetDemoDriver(const UObject* WorldContextObject);

	/**
	 *Set the maximum number of frames recorded per second by the replay
	 */
	UFUNCTION(BlueprintCallable, Category= "ReplaySystem",
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void SetMaxRecordHz(UObject* WorldContextObject, float Hz = 8.0f);


	/**
	* Get the maximum number of frames recorded per second by the replay
	*/
	UFUNCTION(BlueprintCallable, Category= "ReplaySystem")
	static float GetMaxRecordHz();

	/**
	* Serializes A Struct
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Struct"),
		Category= "ReplaySystem")
	static void SerializeStruct(UStruct* Struct, TArray<uint8>& Data);


	/**
	* De-Serializes A Struct
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Struct"),
		Category= "ReplaySystem")
	static void DeSerializeStruct(TArray<uint8> Data, UStruct* & Struct);


	/**
	* Converts A String Into Bytes
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category= "ReplaySystem")
	static TArray<uint8> StringToBytes(FString Data);

	/**
	* Converts Bytes To A String
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category= "ReplaySystem")
	static FString BytesToString(TArray<uint8> Data);


	//Convert property into c++ accessible form
	DECLARE_FUNCTION(execSerializeStruct)
	{
		
		Stack.StepCompiledIn<FStructProperty>(nullptr);
		const auto Struct = CastField<FStructProperty>(Stack.MostRecentProperty);
		const void* StructValue = Stack.MostRecentPropertyAddress;
		
		// Stack.StepCompiledIn<FStructProperty>(nullptr);
		// const auto Data = CastField<FArrayProperty>(Stack.MostRecentProperty);
		// const auto DataValue = Data->ContainerPtrToValuePtr<TArray<uint8>>((void*)Stack.Object);
		PARAM_PASSED_BY_REF(Data, FArrayProperty,TArray<uint8>);
		
		
		
		P_FINISH;

		if(!Struct )//|| !Data)
		{
			return;
		}

		P_NATIVE_BEGIN;

		FString asString = FString();

		FJsonObjectConverter::UStructToJsonObjectString(Struct->Struct, StructValue, asString);

		// DataValue = &(StringToBytes(asString));
		Data = StringToBytes(asString);

		P_NATIVE_END;
	}


	//Convert property into c++ accessible form
	DECLARE_FUNCTION(execDeSerializeStruct)
	{
		// Stack.StepCompiledIn<FStructProperty>(nullptr);
		// const auto Data = CastField<FArrayProperty>(Stack.MostRecentProperty);
		// const auto DataValue = Data->ContainerPtrToValuePtr<TArray<uint8>>((void*)Stack.Object);
		PARAM_PASSED_BY_VAL(Data, FArrayProperty,TArray<uint8>);

		Stack.StepCompiledIn<FStructProperty>(nullptr);
		const auto Struct = CastField<FStructProperty>(Stack.MostRecentProperty);
		void* StructValue = Stack.MostRecentPropertyAddress;

		P_FINISH;

		if(!Struct) //|| !Data)
		{
			return;
		}

		P_NATIVE_BEGIN;

		const auto JsonString = BytesToString(Data);//BytesToString(*DataValue);

		FJsonObjectWrapper Wrapper;

		Wrapper.JsonObjectFromString(JsonString);

		FJsonObjectConverter::JsonObjectToUStruct(Wrapper.JsonObject.ToSharedRef(), Struct->Struct, StructValue);

		P_NATIVE_END;
	}
};

