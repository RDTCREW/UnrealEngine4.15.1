// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
//
#include "HeadMountedDisplay.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "UObject/Package.h"

//#include "UObject/UObjectGlobals.h"

#if !UE_BUILD_SHIPPING
#include "Debug/DebugDrawService.h"
#include "DrawDebugHelpers.h"
#endif

#define LOCTEXT_NAMESPACE "HeadMountedDisplay"
DEFINE_LOG_CATEGORY_STATIC(LogHeadMountedDisplayCommand, Display, All);

/**
* HMD device console vars
*/
static TAutoConsoleVariable<int32> CVarHiddenAreaMask(
	TEXT("vr.HiddenAreaMask"),
	1,
	*LOCTEXT("CVarText_HiddenAreaMask", "Enable or disable hidden area mask\n0: disabled\n1: enabled").ToString(),
	ECVF_Scalability | ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarMirrorMode(
	TEXT("vr.MirrorMode"),
	1,
	*LOCTEXT("CVarText_MirrorMode", 
		"Changes the look of the mirror window if supported by the HMD plugin.\n 0: disable mirroring\n 1: single eye\n 2: stereo pair\nNumbers larger than 2 may be possible and specify HMD plugin-specific variations.\nNegative values are treated the same as 0.").ToString(),
	ECVF_Scalability | ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarEnableDevOverrides(
	TEXT("vr.Debug.bEnableDevOverrides"),
	0,
	*LOCTEXT("CVarText_EnableDevOverrides", "Enables or disables console commands that modify various developer-only settings.").ToString());


#if !UE_BUILD_SHIPPING
static void DrawDebugTrackingSensorLocations(UCanvas* Canvas, APlayerController* PlayerController, UWorld* World)
{
	if (!GEngine || !GEngine->HMDDevice.IsValid() || !GEngine->HMDDevice->IsStereoEnabled())
	{
		return;
	}
	if (!PlayerController)
	{
		PlayerController = World->GetFirstPlayerController();
		if (!PlayerController)
		{
			return;
		}
	}
	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		return;
	}
	FVector ViewLocation = Pawn->GetPawnViewLocation();
	FRotator ViewRotation = Pawn->GetViewRotation();

	const FColor FrustrumColor = (GEngine->HMDDevice->HasValidTrackingPosition() ? FColor::Green : FColor::Red);
	FVector SensorOrigin;
	FQuat SensorOrient;
	float LFovDeg, RFovDeg, TFovDeg, BFovDeg, NearPlane, FarPlane, CameraDist;
	uint32 NumSensors = GEngine->HMDDevice->GetNumOfTrackingSensors();
	FVector HeadPosition;
	FQuat HeadOrient;
	GEngine->HMDDevice->GetCurrentOrientationAndPosition(HeadOrient, HeadPosition);
	const FQuat DeltaControlOrientation = ViewRotation.Quaternion() * HeadOrient.Inverse();
	for (uint8 SensorIndex = 0; SensorIndex < NumSensors; ++SensorIndex)
	{

		GEngine->HMDDevice->GetTrackingSensorProperties(SensorIndex, SensorOrigin, SensorOrient, LFovDeg, RFovDeg, TFovDeg, BFovDeg, CameraDist, NearPlane, FarPlane);


		SensorOrient = DeltaControlOrientation * SensorOrient;
		SensorOrigin = DeltaControlOrientation.RotateVector(SensorOrigin);

		// Calculate the edge vectors of the pyramid from the FoV angles
		const float LeftTan = -FMath::Tan(FMath::DegreesToRadians(LFovDeg));
		const float RightTan = FMath::Tan(FMath::DegreesToRadians(RFovDeg));
		const float TopTan = FMath::Tan(FMath::DegreesToRadians(TFovDeg));
		const float BottomTan = -FMath::Tan(FMath::DegreesToRadians(BFovDeg));
		FVector EdgeTR(1, RightTan, TopTan);
		FVector EdgeTL(1, LeftTan, TopTan);
		FVector EdgeBL(1, LeftTan, BottomTan);
		FVector EdgeBR(1, RightTan, BottomTan);

		// Create a matrix to translate from sensor-relative coordinates to the view location
		FMatrix Matrix = SensorOrient * FMatrix::Identity;
		Matrix *= FTranslationMatrix(SensorOrigin);
		Matrix *= FTranslationMatrix(ViewLocation);

		// Calculate coordinates of the tip (location of the sensor) and the base of the pyramid (far plane)
		FVector Tip = Matrix.TransformPosition(FVector::ZeroVector);
		FVector BaseTR = Matrix.TransformPosition(EdgeTR * FarPlane);
		FVector BaseTL = Matrix.TransformPosition(EdgeTL * FarPlane);
		FVector BaseBL = Matrix.TransformPosition(EdgeBL * FarPlane);
		FVector BaseBR = Matrix.TransformPosition(EdgeBR * FarPlane);

		// Calculate coordinates of where the near plane intersects the pyramid
		FVector NearTR = Matrix.TransformPosition(EdgeTR * NearPlane);
		FVector NearTL = Matrix.TransformPosition(EdgeTL * NearPlane);
		FVector NearBL = Matrix.TransformPosition(EdgeBL * NearPlane);
		FVector NearBR = Matrix.TransformPosition(EdgeBR * NearPlane);

		// Draw a point at the sensor position
		DrawDebugPoint(World, Tip, 5, FrustrumColor);

		// Draw the four edges of the pyramid
		DrawDebugLine(World, Tip, BaseTR, FrustrumColor);
		DrawDebugLine(World, Tip, BaseTL, FrustrumColor);
		DrawDebugLine(World, Tip, BaseBL, FrustrumColor);
		DrawDebugLine(World, Tip, BaseBR, FrustrumColor);

		// Draw the base (far plane)
		DrawDebugLine(World, BaseTR, BaseTL, FrustrumColor);
		DrawDebugLine(World, BaseTL, BaseBL, FrustrumColor);
		DrawDebugLine(World, BaseBL, BaseBR, FrustrumColor);
		DrawDebugLine(World, BaseBR, BaseTR, FrustrumColor);

		// Draw the near plane
		DrawDebugLine(World, NearTR, NearTL, FrustrumColor);
		DrawDebugLine(World, NearTL, NearBL, FrustrumColor);
		DrawDebugLine(World, NearBL, NearBR, FrustrumColor);
		DrawDebugLine(World, NearBR, NearTR, FrustrumColor);

		// Draw a center line from the sensor to the focal point
		FVector CenterLine = Matrix.TransformPosition(FVector(CameraDist, 0, 0));
		DrawDebugLine(World, Tip, CenterLine, FColor::Yellow);
		DrawDebugPoint(World, CenterLine, 5, FColor::Yellow);
	}
}

static void ShowTrackingSensors(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar)
{
	static FDelegateHandle Handle;
	if (Args.Num())
	{
		bool bShouldEnable = FCString::ToBool(*Args[0]);
		if (Handle.IsValid() != bShouldEnable)
		{
			if (bShouldEnable)
			{
				Handle = UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateStatic(DrawDebugTrackingSensorLocations, World));
			}
			else
			{
				UDebugDrawService::Unregister(Handle);
				Handle.Reset();
			}
		}
	}
	Ar.Logf(TEXT("Tracking sensor drawing is %s"), Handle.IsValid() ? TEXT("enabled") : TEXT("disabled"));
}

static FAutoConsoleCommand CShowTrackingSensorsCmd(
	TEXT("vr.Debug.VisualizeTrackingSensors"),
	*LOCTEXT("CVarText_ShowTrackingSensors", "Show or hide the location and coverage area of the tracking sensors\nUse 1, True, or Yes to enable, 0, False or No to disable.").ToString(),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(ShowTrackingSensors));
#endif

static void TrackingOrigin(const TArray<FString>& Args, UWorld* , FOutputDevice& Ar)
{
	const static UEnum* TrackingOriginEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EHMDTrackingOrigin"));
	int Origin = INDEX_NONE;
	if (Args.Num())
	{
		if (FCString::IsNumeric(*Args[0]))
		{
			Origin = FCString::Atoi(*Args[0]);
		}
		else
		{
			Origin = TrackingOriginEnum->GetIndexByName(*Args[0]);
		}
		if (Origin < 0 || Origin > 1)
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("Invalid tracking orgin, %s"), *Args[0]);
		}

		if (GEngine && GEngine->HMDDevice.IsValid())
		{
			GEngine->HMDDevice->SetTrackingOrigin(EHMDTrackingOrigin::Type(Origin));
		}
	}
	else
	{
		if (GEngine && GEngine->HMDDevice.IsValid())
		{
			Origin = GEngine->HMDDevice->GetTrackingOrigin();
		}
		Ar.Logf(TEXT("Tracking orgin is set to %s"), *TrackingOriginEnum->GetNameStringByIndex(Origin));
	}
}

static FAutoConsoleCommand CTrackingOriginCmd(
	TEXT("vr.TrackingOrigin"),
	*LOCTEXT("CCommandText_TrackingOrigin", "Floor or 0 - tracking origin is at the floor, Eye or 1 - tracking origin is at the eye level.").ToString(),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(TrackingOrigin));

static void HMDResetPosition(const TArray<FString>& Args, UWorld*, FOutputDevice& Ar)
{
	if (GEngine && GEngine->HMDDevice.IsValid())
	{
		GEngine->HMDDevice->ResetPosition();
	}
}

static FAutoConsoleCommand CHMDResetPositionCmd(
	TEXT("vr.HeadTracking.ResetPosition"),
	*LOCTEXT("CVarText_HMDResetPosition", "Reset the position of the head mounted display.").ToString(),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(HMDResetPosition));


static void HMDResetOrientation(const TArray<FString>& Args, UWorld*, FOutputDevice& Ar)
{
	if (GEngine && GEngine->HMDDevice.IsValid())
	{
		float Yaw = 0.f;
		if (Args.Num() > 0)
		{
			Yaw = FCString::Atof(*Args[0]);
		}
		GEngine->HMDDevice->ResetOrientation(Yaw);
	}
}

static FAutoConsoleCommand CHMDResetOrientationCmd(
	TEXT("vr.HeadTracking.ResetOrientation"),
	*LOCTEXT("CVarText_HMDResetOrientation", "Reset the rotation of the head mounted display.\nPass in an optional yaw for the new rotation in degrees .").ToString(),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(HMDResetOrientation));

static void HMDReset(const TArray<FString>& Args, UWorld*, FOutputDevice& Ar)
{
	if (GEngine && GEngine->HMDDevice.IsValid())
	{
		float Yaw = 0.f;
		if (Args.Num() > 0)
		{
			Yaw = FCString::Atof(*Args[0]);
		}
		GEngine->HMDDevice->ResetOrientationAndPosition(Yaw);
	}
}

static FAutoConsoleCommand CHMDResetCmd(
	TEXT("vr.HeadTracking.Reset"),
	*LOCTEXT("CVarText_HMDReset", "Reset the rotation and position of the head mounted display.\nPass in an optional yaw for the new rotation in degrees.").ToString(),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(HMDReset));


static void HMDStatus(const TArray<FString>& Args, UWorld*, FOutputDevice& Ar)
{
	if (GEngine && GEngine->HMDDevice.IsValid())
	{
		auto HMD = GEngine->HMDDevice;
		Ar.Logf(TEXT("Position tracking status: %s\nHead tracking allowed: %s\nNumber of tracking sensors: %d"), 
			HMD->DoesSupportPositionalTracking() ? (HMD->HasValidTrackingPosition() ? TEXT("active") : TEXT("lost")) : TEXT("not supported"),
			HMD->IsHeadTrackingAllowed() ? TEXT("yes") : TEXT("no"),
			HMD->GetNumOfTrackingSensors());
	}
}

static FAutoConsoleCommand CHMDStatusCmd(
	TEXT("vr.HeadTracking.Status"),
	*LOCTEXT("CVarText_HMDStatus", "Reports the current status of the head tracking.").ToString(),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(HMDStatus));

static void EnableHMD(const TArray<FString>& Args, UWorld*, FOutputDevice& Ar)
{
	bool bEnable = 0;
	if (Args.Num())
	{
		bEnable = FCString::ToBool(*Args[0]);

		if (GEngine && GEngine->HMDDevice.IsValid())
		{
			GEngine->HMDDevice->EnableHMD(bEnable);
		}
	}
	else
	{
		if (GEngine && GEngine->HMDDevice.IsValid())
		{
			bEnable = GEngine->HMDDevice->IsHMDEnabled();
		}
		Ar.Logf(TEXT("HMD device is %s"), bEnable?TEXT("enabled"):TEXT("disabled"));
	}
}

static FAutoConsoleCommand CEnableHMDCmd(
	TEXT("vr.bEnableHMD"),
	*LOCTEXT("CCommandText_EnableHMD", "Enables or disables the HMD device. Use 1, True, or Yes to enable, 0, False or No to disable.").ToString(),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(EnableHMD));

static void EnableStereo(const TArray<FString>& Args, UWorld*, FOutputDevice& Ar)
{
	bool bEnable = 0;
	if (Args.Num())
	{
		bEnable = FCString::ToBool(*Args[0]);

		if (GEngine && GEngine->HMDDevice.IsValid())
		{
			if (!GEngine->HMDDevice->IsHMDEnabled())
			{
				Ar.Logf(TEXT("HMD is disabled. Use 'vr.bEnableHMD True' to re-enable it."));
			}
			GEngine->HMDDevice->EnableStereo(bEnable);
		}
	}
	else
	{
		if (GEngine && GEngine->HMDDevice.IsValid())
		{
			bEnable = GEngine->HMDDevice->IsStereoEnabled();
		}
		Ar.Logf(TEXT("Stereo is %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
	}
}

static FAutoConsoleCommand CEnableStereoCmd(
	TEXT("vr.bEnableStereo"),
	*LOCTEXT("CCommandText_EnableStereo", "Enables or disables the stereo rendering. Use 1, True, or Yes to enable, 0, False or No to disable.").ToString(),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(EnableStereo));

static void HMDVersion(const TArray<FString>& Args, UWorld*, FOutputDevice& Ar)
{
	if (GEngine && GEngine->HMDDevice.IsValid())
	{
		Ar.Logf(*GEngine->HMDDevice->GetVersionString());
	}
}

static FAutoConsoleCommand CHMDVersionCmd(
	TEXT("vr.HMDVersion"),
	*LOCTEXT("CCommandText_HMDVersion", "Prints version information for the current HMD device.").ToString(),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(HMDVersion));

static void WorldToMeters(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar)
{
	if (Args.Num())
	{
		float WorldToMeters = -1;
		if (FCString::IsNumeric(*Args[0]))
		{
			WorldToMeters = FCString::Atof(*Args[0]);
		}

		if (WorldToMeters <= 0)
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("Invalid argument, %s. World to meters scale must be larger than 0."), *Args[0]);
		}
		else
		{
			World->GetWorldSettings()->WorldToMeters = WorldToMeters;
		}
	}
	else
	{
		Ar.Logf(TEXT("World to meters scale is %0.2f"), World->GetWorldSettings()->WorldToMeters);
	}
}

static FAutoConsoleCommand CWorldToMetersCmd(
	TEXT("vr.WorldToMetersScale"),
	*LOCTEXT("CCommandText_WorldToMeters", 
		"Get or set the current world to meters scale.").ToString(),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(WorldToMeters));

/**
* Exec handler that aliases old deprecated VR console commands to the new ones.
*
* @param InWorld World context
* @param Cmd 	the exec command being executed
* @param Ar 	the archive to log results to
*
* @return true if the handler consumed the input, false to continue searching handlers
*/
static bool CompatExec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	const TCHAR* OrigCmd = Cmd;
	FString AliasedCommand;
	if (FParse::Command(&Cmd, TEXT("vr.SetTrackingOrigin")))
	{
		AliasedCommand = FString::Printf(TEXT("vr.TrackingOrigin %s"), Cmd);
	}
	else if (FParse::Command(&Cmd, TEXT("HMDPOS")))
	{
		FString CmdName = FParse::Token(Cmd, 0);
		if (CmdName.Equals(TEXT("EYE"), ESearchCase::IgnoreCase) || CmdName.Equals(TEXT("FLOOR"), ESearchCase::IgnoreCase))
		{
			AliasedCommand = FString::Printf(TEXT("vr.TrackingOrigin %s"), *CmdName);
		}
	}
	else if (FParse::Command(&Cmd, TEXT("HMD")))
	{
		if (FParse::Command(&Cmd, TEXT("ON")) || FParse::Command(&Cmd, TEXT("ENABLE")))
		{
			AliasedCommand = TEXT("vr.bEnableHMD True");
		}
		if (FParse::Command(&Cmd, TEXT("OFF")) || FParse::Command(&Cmd, TEXT("DISABLE")))
		{
			AliasedCommand = TEXT("vr.bEnableHMD False");
		}
		if (FParse::Command(&Cmd, TEXT("SP")) || FParse::Command(&Cmd, TEXT("SCREENPERCENTAGE")))
		{
			AliasedCommand = FString::Printf(TEXT("r.ScreenPercentage %s"), Cmd);
		}
	}
	else if (FParse::Command(&Cmd, TEXT("STEREO")))
	{
		FString Value;
		if (FParse::Command(&Cmd, TEXT("ON")) || FParse::Command(&Cmd, TEXT("ENABLE")))
		{
			AliasedCommand = TEXT("vr.bEnableStereo True");
		}
		else if (FParse::Command(&Cmd, TEXT("OFF")) || FParse::Command(&Cmd, TEXT("DISABLE")))
		{
			AliasedCommand = TEXT("vr.bEnableStereo False");
		}
		else if (FParse::Value(Cmd, TEXT("W2M="), Value))
		{
			AliasedCommand = FString::Printf(TEXT("vr.WorldToMetersScale %s"), *Value);
		}
	}
	else if (FParse::Command(&Cmd, TEXT("HMDVERSION")))
	{
		AliasedCommand = TEXT("vr.HMDVersion");
	}

	if (!AliasedCommand.IsEmpty())
	{
		Ar.Logf(ELogVerbosity::Warning, TEXT("%s is deprecated. Use %s instead"), OrigCmd, *AliasedCommand);

		return IConsoleManager::Get().ProcessUserConsoleInput(*AliasedCommand, Ar, InWorld);
	}
	else
	{
		return false;
	}
}

static FStaticSelfRegisteringExec CompatExecRegistration(CompatExec);
