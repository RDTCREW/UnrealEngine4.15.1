// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "AnimNodes/AnimNode_AimOffsetLookAt.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "Animation/BlendSpaceBase.h"
#include "Animation/BlendSpace1D.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"

TAutoConsoleVariable<int32> CVarAimOffsetLookAtEnable(TEXT("a.AnimNode.AimOffsetLookAt.Enable"), 1, TEXT("Enable/Disable LookAt AimOffset"));
TAutoConsoleVariable<int32> CVarAimOffsetLookAtDebug(TEXT("a.AnimNode.AimOffsetLookAt.Debug"), 0, TEXT("Toggle LookAt AimOffset debug"));

/////////////////////////////////////////////////////
// FAnimNode_AimOffsetLookAt

void FAnimNode_AimOffsetLookAt::Initialize(const FAnimationInitializeContext& Context)
{
	FAnimNode_BlendSpacePlayer::Initialize(Context);
	BasePose.Initialize(Context);
}

void FAnimNode_AimOffsetLookAt::RootInitialize(const FAnimInstanceProxy* InProxy)
{
	FAnimNode_BlendSpacePlayer::RootInitialize(InProxy);

	SocketBoneReference.BoneName = NAME_None;
	if (USkeletalMeshComponent* SkelMeshComp = InProxy->GetSkelMeshComponent())
	{
		if (USkeletalMesh* SkelMesh = SkelMeshComp->SkeletalMesh)
		{
			if (const USkeletalMeshSocket* Socket = SkelMesh->FindSocket(SourceSocketName))
			{
				SocketLocalTransform = Socket->GetSocketLocalTransform();
				SocketBoneReference.BoneName = Socket->BoneName;
			}

			if (const USkeletalMeshSocket* Socket = SkelMesh->FindSocket(PivotSocketName))
			{
				PivotSocketLocalTransform = Socket->GetSocketLocalTransform();
				PivotSocketBoneReference.BoneName = Socket->BoneName;
			}
		}
	}
}

void FAnimNode_AimOffsetLookAt::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	EvaluateGraphExposedInputs.Execute(Context);

	bIsLODEnabled = IsLODEnabled(Context.AnimInstanceProxy, LODThreshold);

	// We don't support ticking and advancing time, because Inputs are determined during Evaluate.
	// it may be possible to advance time there (is it a problem with notifies?)
	// But typically AimOffsets contain single frame poses, so time doesn't matter.

// 	if (bIsLODEnabled)
// 	{
// 		FAnimNode_BlendSpacePlayer::UpdateAssetPlayer(Context);
// 	}

	BasePose.Update(Context);
}

void FAnimNode_AimOffsetLookAt::CacheBones(const FAnimationCacheBonesContext& Context)
{
	FAnimNode_BlendSpacePlayer::CacheBones(Context);
	BasePose.CacheBones(Context);

	SocketBoneReference.Initialize(Context.AnimInstanceProxy->GetRequiredBones());
	PivotSocketBoneReference.Initialize(Context.AnimInstanceProxy->GetRequiredBones());
}

void FAnimNode_AimOffsetLookAt::Evaluate(FPoseContext& Context)
{
	// Evaluate base pose
	BasePose.Evaluate(Context);

	if (bIsLODEnabled && FAnimWeight::IsRelevant(Alpha) && (CVarAimOffsetLookAtEnable.GetValueOnAnyThread() == 1))
	{
		UpdateFromLookAtTarget(Context);

		// Evaluate MeshSpaceRotation additive blendspace
		FPoseContext MeshSpaceRotationAdditivePoseContext(Context);
		FAnimNode_BlendSpacePlayer::Evaluate(MeshSpaceRotationAdditivePoseContext);

		// Accumulate poses together
		FAnimationRuntime::AccumulateMeshSpaceRotationAdditiveToLocalPose(Context.Pose, MeshSpaceRotationAdditivePoseContext.Pose, Context.Curve, MeshSpaceRotationAdditivePoseContext.Curve, Alpha);

		// Resulting rotations are not normalized, so normalize here.
		Context.Pose.NormalizeRotations();
	}
}

void FAnimNode_AimOffsetLookAt::UpdateFromLookAtTarget(FPoseContext& LocalPoseContext)
{
	FVector BlendInput(X, Y, Z);

	const FBoneContainer& RequiredBones = LocalPoseContext.Pose.GetBoneContainer();
	if (BlendSpace && SocketBoneReference.IsValid(RequiredBones))
	{
		FCSPose<FCompactPose> GlobalPose;
		GlobalPose.InitPose(LocalPoseContext.Pose);

		const FCompactPoseBoneIndex SocketBoneIndex = SocketBoneReference.GetCompactPoseIndex(RequiredBones);
		const FTransform BoneTransform = GlobalPose.GetComponentSpaceTransform(SocketBoneIndex);

		FTransform SourceComponentTransform = SocketLocalTransform * BoneTransform;
		if (PivotSocketBoneReference.IsValid(RequiredBones))
		{
			const FCompactPoseBoneIndex PivotSocketBoneIndex = PivotSocketBoneReference.GetCompactPoseIndex(RequiredBones);
			const FTransform PivotBoneComponentTransform = GlobalPose.GetComponentSpaceTransform(PivotSocketBoneIndex);

			SourceComponentTransform.SetTranslation(PivotBoneComponentTransform.GetTranslation());
		}

		FAnimInstanceProxy* AnimProxy = LocalPoseContext.AnimInstanceProxy;
		check(AnimProxy);
		const FTransform SourceWorldTransform = SourceComponentTransform * AnimProxy->GetSkelMeshCompLocalToWorld();
		const FTransform ActorTransform = AnimProxy->GetSkelMeshCompOwnerTransform();

		// Convert Target to Actor Space
		const FTransform TargetWorldTransform(LookAtLocation);

		const FVector DirectionToTarget = ActorTransform.InverseTransformVectorNoScale(TargetWorldTransform.GetLocation() - SourceWorldTransform.GetLocation()).GetSafeNormal();
		const FVector CurrentDirection = ActorTransform.InverseTransformVectorNoScale(SourceWorldTransform.GetUnitAxis(EAxis::X));

		const FVector AxisX = FVector::ForwardVector;
		const FVector AxisY = FVector::RightVector;
		const FVector AxisZ = FVector::UpVector;

		const FVector2D CurrentCoords = FMath::GetAzimuthAndElevation(CurrentDirection, AxisX, AxisY, AxisZ);
		const FVector2D TargetCoords = FMath::GetAzimuthAndElevation(DirectionToTarget, AxisX, AxisY, AxisZ);
		BlendInput.X = FRotator::NormalizeAxis(FMath::RadiansToDegrees(TargetCoords.X - CurrentCoords.X));
		BlendInput.Y = FRotator::NormalizeAxis(FMath::RadiansToDegrees(TargetCoords.Y - CurrentCoords.Y));

#if ENABLE_DRAW_DEBUG
		if (CVarAimOffsetLookAtDebug.GetValueOnAnyThread() == 1)
		{
			AnimProxy->AnimDrawDebugLine(SourceWorldTransform.GetLocation(), TargetWorldTransform.GetLocation(), FColor::Green);
			AnimProxy->AnimDrawDebugLine(SourceWorldTransform.GetLocation(), SourceWorldTransform.GetLocation() + SourceWorldTransform.GetUnitAxis(EAxis::X) * (TargetWorldTransform.GetLocation() - SourceWorldTransform.GetLocation()).Size(), FColor::Red);
			AnimProxy->AnimDrawDebugCoordinateSystem(ActorTransform.GetLocation(), ActorTransform.GetRotation().Rotator(), 100.f);

			FString DebugString = FString::Printf(TEXT("Socket (X:%f, Y:%f), Target (X:%f, Y:%f), Result (X:%f, Y:%f)")
				, FMath::RadiansToDegrees(CurrentCoords.X)
				, FMath::RadiansToDegrees(CurrentCoords.Y)
				, FMath::RadiansToDegrees(TargetCoords.X)
				, FMath::RadiansToDegrees(TargetCoords.Y)
				, BlendInput.X
				, BlendInput.Y);
			AnimProxy->AnimDrawDebugOnScreenMessage(DebugString, FColor::Red);
		}
#endif // ENABLE_DRAW_DEBUG
	}

	// Set X and Y, so ticking next frame is based on correct weights.
	X = BlendInput.X;
	Y = BlendInput.Y;

	// Generate BlendSampleDataCache from inputs.
	if (BlendSpace)
	{
		BlendSpace->GetSamplesFromBlendInput(BlendInput, BlendSampleDataCache);
	}
}

void FAnimNode_AimOffsetLookAt::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugLine += FString::Printf(TEXT("(Play Time: %.3f)"), InternalTimeAccumulator);
	DebugData.AddDebugItem(DebugLine);

	BasePose.GatherDebugData(DebugData);
}

FAnimNode_AimOffsetLookAt::FAnimNode_AimOffsetLookAt()
	: LODThreshold(INDEX_NONE)
	, Alpha(1.f)
{
}
