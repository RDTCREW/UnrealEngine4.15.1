// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	AtmosphereRendering.cpp: Fog rendering implementation.
=============================================================================*/

#include "RendererPrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"
#include "AtmosphereRendering.h"
#include "../../Engine/Private/Atmosphere/Atmosphere.h"
#include "AtmosphereTextures.h"

class FAtmosphereShaderPrecomputeTextureParameters
{
public:
	enum TexType
	{
		Transmittance = 0,
		Irradiance,
		DeltaE,
		Inscatter,
		DeltaSR,
		DeltaSM,
		DeltaJ,
		Type_MAX
	};

	void Bind(const FShaderParameterMap& ParameterMap, uint32 TextureIdx, TexType TextureType)
	{
		switch(TextureType)
		{
		case Transmittance:
			AtmosphereTexture[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereTransmittanceTexture"));
			AtmosphereTextureSampler[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereTransmittanceTextureSampler"));
			break;
		case Irradiance:
			AtmosphereTexture[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereIrradianceTexture"));
			AtmosphereTextureSampler[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereIrradianceTextureSampler"));
			break;
		case Inscatter:
			AtmosphereTexture[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereInscatterTexture"));
			AtmosphereTextureSampler[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereInscatterTextureSampler"));
			break;
		case DeltaE:
			AtmosphereTexture[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereDeltaETexture"));
			AtmosphereTextureSampler[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereDeltaETextureSampler"));
			break;
		case DeltaSR:
			AtmosphereTexture[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereDeltaSRTexture"));
			AtmosphereTextureSampler[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereDeltaSRTextureSampler"));
			break;
		case DeltaSM:
			AtmosphereTexture[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereDeltaSMTexture"));
			AtmosphereTextureSampler[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereDeltaSMTextureSampler"));
			break;
		case DeltaJ:
			AtmosphereTexture[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereDeltaJTexture"));
			AtmosphereTextureSampler[TextureIdx].Bind(ParameterMap,TEXT("AtmosphereDeltaJTextureSampler"));
			break;
		}
	}

	template<typename ShaderRHIParamRef>
	void Set(FRHICommandList& RHICmdList, const ShaderRHIParamRef ShaderRHI, uint32 TextureIdx, const FTextureRHIRef Texture) const
	{
		if (TextureIdx >= 4)
		{
			return;
		}

		SetTextureParameter(RHICmdList, ShaderRHI, AtmosphereTexture[TextureIdx], AtmosphereTextureSampler[TextureIdx], 
			TStaticSamplerState<SF_Bilinear>::GetRHI(), 
			Texture);
	}

	template<typename ShaderRHIParamRef>
	void Set(FRHICommandList& RHICmdList, const ShaderRHIParamRef ShaderRHI, uint32 TextureIdx, TexType TextureType, const FAtmosphereTextures* AtmosphereTextures) const
	{
		if (TextureIdx >= 4 || TextureType >= Type_MAX || !AtmosphereTextures)
		{
			return;
		}

		switch(TextureType)
		{
		case Transmittance:
			SetTextureParameter(RHICmdList, ShaderRHI, AtmosphereTexture[TextureIdx], AtmosphereTextureSampler[TextureIdx], 
				TStaticSamplerState<SF_Bilinear>::GetRHI(), 
				AtmosphereTextures->AtmosphereTransmittance->GetRenderTargetItem().ShaderResourceTexture);
			break;
		case Irradiance:
			SetTextureParameter(RHICmdList, ShaderRHI, AtmosphereTexture[TextureIdx], AtmosphereTextureSampler[TextureIdx], 
				TStaticSamplerState<SF_Bilinear>::GetRHI(), 
				AtmosphereTextures->AtmosphereIrradiance->GetRenderTargetItem().ShaderResourceTexture);
			break;
		case DeltaE:
			SetTextureParameter(RHICmdList, ShaderRHI, AtmosphereTexture[TextureIdx], AtmosphereTextureSampler[TextureIdx], 
				TStaticSamplerState<SF_Bilinear>::GetRHI(), 
				AtmosphereTextures->AtmosphereDeltaE->GetRenderTargetItem().ShaderResourceTexture);
			break;
		case Inscatter:
			SetTextureParameter(RHICmdList, ShaderRHI, AtmosphereTexture[TextureIdx], AtmosphereTextureSampler[TextureIdx], 
				TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), 
				AtmosphereTextures->AtmosphereInscatter->GetRenderTargetItem().ShaderResourceTexture);
			break;
		case DeltaSR:
			SetTextureParameter(RHICmdList, ShaderRHI, AtmosphereTexture[TextureIdx], AtmosphereTextureSampler[TextureIdx], 
				TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), 
				AtmosphereTextures->AtmosphereDeltaSR->GetRenderTargetItem().ShaderResourceTexture);
			break;
		case DeltaSM:
			SetTextureParameter(RHICmdList, ShaderRHI, AtmosphereTexture[TextureIdx], AtmosphereTextureSampler[TextureIdx], 
				TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), 
				AtmosphereTextures->AtmosphereDeltaSM->GetRenderTargetItem().ShaderResourceTexture);
			break;
		case DeltaJ:
			SetTextureParameter(RHICmdList, ShaderRHI, AtmosphereTexture[TextureIdx], AtmosphereTextureSampler[TextureIdx], 
				TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), 
				AtmosphereTextures->AtmosphereDeltaJ->GetRenderTargetItem().ShaderResourceTexture);
			break;
		}
	}

	friend FArchive& operator<<(FArchive& Ar,FAtmosphereShaderPrecomputeTextureParameters& P);

private:
	FShaderResourceParameter AtmosphereTexture[4];
	FShaderResourceParameter AtmosphereTextureSampler[4];
};

FArchive& operator<<(FArchive& Ar,FAtmosphereShaderPrecomputeTextureParameters& Parameters)
{
	for (uint32 i = 0; i < 4; ++i)
	{
		Ar << Parameters.AtmosphereTexture[i];
		Ar << Parameters.AtmosphereTextureSampler[i];
	}
	return Ar;
}

/** A pixel shader for rendering atmospheric fog. */
class FAtmosphericFogPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphericFogPS, Global);
public:
	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/**
	* Add any compiler flags/defines required by the shader
	* @param OutEnvironment - shader environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Platform, OutEnvironment);
	}

	FAtmosphericFogPS() {}
	FAtmosphericFogPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		SceneTextureParameters.Bind(Initializer.ParameterMap);
		AtmosphereTextureParameters.Bind(Initializer.ParameterMap);
		OcclusionTextureParameter.Bind(Initializer.ParameterMap, TEXT("OcclusionTexture"));
		OcclusionTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("OcclusionTextureSampler"));
	}

	void SetParameters(FRHICommandList& RHICmdList, const FSceneView& View, TRefCountPtr<IPooledRenderTarget>& LightShaftOcclusion)
	{
		FGlobalShader::SetParameters(RHICmdList, GetPixelShader(), View);
		SceneTextureParameters.Set(RHICmdList, GetPixelShader(), View);
		AtmosphereTextureParameters.Set(RHICmdList, GetPixelShader(), View);

		if (OcclusionTextureParameter.IsBound())
		{
			if (LightShaftOcclusion)
			{
				SetTextureParameter(
					RHICmdList, 
					GetPixelShader(),
					OcclusionTextureParameter, OcclusionTextureSamplerParameter,
					TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
					LightShaftOcclusion->GetRenderTargetItem().ShaderResourceTexture
					);
			}
			else
			{
				SetTextureParameter(
					RHICmdList, 
					GetPixelShader(),
					OcclusionTextureParameter, OcclusionTextureSamplerParameter,
					TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
					GWhiteTexture->TextureRHI
					);
			}
		}
	}

	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << SceneTextureParameters;
		Ar << AtmosphereTextureParameters;
		Ar << OcclusionTextureParameter;
		Ar << OcclusionTextureSamplerParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FSceneTextureShaderParameters SceneTextureParameters;
	FAtmosphereShaderTextureParameters AtmosphereTextureParameters;
	FShaderResourceParameter OcclusionTextureParameter;
	FShaderResourceParameter OcclusionTextureSamplerParameter;
};

template<uint32 RenderFlag>
class TAtmosphericFogPS : public FAtmosphericFogPS
{
	DECLARE_SHADER_TYPE(TAtmosphericFogPS, Global);

	/** Default constructor. */
	TAtmosphericFogPS() {}
public:
	TAtmosphericFogPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FAtmosphericFogPS(Initializer)
	{
	}

	/**
	* Add any compiler flags/defines required by the shader
	* @param OutEnvironment - shader environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		FAtmosphericFogPS::ModifyCompilationEnvironment(Platform, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("ATMOSPHERIC_NO_SUN_DISK"), (RenderFlag & EAtmosphereRenderFlag::E_DisableSunDisk) ? TEXT("1") : TEXT("0"));
		OutEnvironment.SetDefine(TEXT("ATMOSPHERIC_NO_GROUND_SCATTERING"), (RenderFlag & EAtmosphereRenderFlag::E_DisableGroundScattering) ? TEXT("1") : TEXT("0"));
		OutEnvironment.SetDefine(TEXT("ATMOSPHERIC_NO_LIGHT_SHAFT"), (RenderFlag & EAtmosphereRenderFlag::E_DisableLightShaft) ? TEXT("1") : TEXT("0"));
	}
};

#define SHADER_VARIATION(RenderFlag) IMPLEMENT_SHADER_TYPE(template<>,TAtmosphericFogPS<RenderFlag>, TEXT("AtmosphericFogShader"), TEXT("AtmosphericPixelMain"), SF_Pixel);
SHADER_VARIATION(EAtmosphereRenderFlag::E_EnableAll)
SHADER_VARIATION(EAtmosphereRenderFlag::E_DisableSunDisk)
SHADER_VARIATION(EAtmosphereRenderFlag::E_DisableGroundScattering)
SHADER_VARIATION(EAtmosphereRenderFlag::E_DisableSunAndGround)
SHADER_VARIATION(EAtmosphereRenderFlag::E_DisableLightShaft)
SHADER_VARIATION(EAtmosphereRenderFlag::E_DisableSunAndLightShaft)
SHADER_VARIATION(EAtmosphereRenderFlag::E_DisableGroundAndLightShaft)
SHADER_VARIATION(EAtmosphereRenderFlag::E_DisableAll)
#undef SHADER_VARIATION

/** The fog vertex declaration resource type. */
class FAtmopshereVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	// Destructor
	virtual ~FAtmopshereVertexDeclaration() {}

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		Elements.Add(FVertexElement(0, 0, VET_Float2, 0, sizeof(FVector2D)));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/** A vertex shader for rendering height fog. */
class FAtmosphericVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphericVS,Global);
public:

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM3);
	}

	FAtmosphericVS( )	{ }
	FAtmosphericVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FGlobalShader(Initializer)
	{
	}

	void SetParameters(FRHICommandList& RHICmdList, const FViewInfo& View)
	{
		FGlobalShader::SetParameters(RHICmdList, GetVertexShader(),View);
	}

	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		return bShaderHasOutdatedParameters;
	}
};

IMPLEMENT_SHADER_TYPE(,FAtmosphericVS,TEXT("AtmosphericFogShader"),TEXT("VSMain"),SF_Vertex);

/** Vertex declaration for the light function fullscreen 2D quad. */
TGlobalResource<FAtmopshereVertexDeclaration> GAtmophereVertexDeclaration;

void FSceneRenderer::InitAtmosphereConstants()
{
	for(int32 ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		bool bInitTextures = false;
		FViewInfo& View = Views[ViewIndex];
		// set fog consts based on height fog components
		if(ShouldRenderAtmosphere(*View.Family))
		{
			if (Scene->AtmosphericFog)
			{
				const FAtmosphericFogSceneInfo& FogInfo = *Scene->AtmosphericFog;

				if (FogInfo.Component->PrecomputeCounter.GetValue() >= UAtmosphericFogComponent::EValid)
				{
					View.AtmosphereTransmittanceTexture = FogInfo.Component->TransmittanceResource
						? (FTextureRHIRef)FogInfo.Component->TransmittanceResource->TextureRHI
						: (FogInfo.Component->TransmittanceTexture_DEPRECATED && FogInfo.Component->TransmittanceTexture_DEPRECATED->Resource) 
								? FogInfo.Component->TransmittanceTexture_DEPRECATED->Resource->TextureRHI  : GBlackTexture->TextureRHI;  
					View.AtmosphereIrradianceTexture = FogInfo.Component->IrradianceResource
						? (FTextureRHIRef)FogInfo.Component->IrradianceResource->TextureRHI
						: (FogInfo.Component->IrradianceTexture_DEPRECATED && FogInfo.Component->IrradianceTexture_DEPRECATED->Resource)
								? FogInfo.Component->IrradianceTexture_DEPRECATED->Resource->TextureRHI : GBlackTexture->TextureRHI;
					View.AtmosphereInscatterTexture = FogInfo.Component->InscatterResource 
						? FogInfo.Component->InscatterResource->TextureRHI : (FTextureRHIRef)GBlackVolumeTexture->TextureRHI;

					bInitTextures = true;
				}
			}
		}

		if (!bInitTextures)
		{
			View.AtmosphereTransmittanceTexture = GBlackTexture->TextureRHI;
			View.AtmosphereIrradianceTexture = GBlackTexture->TextureRHI;
			View.AtmosphereInscatterTexture = GBlackVolumeTexture->TextureRHI;
		}
	}
}

FGlobalBoundShaderState AtmosphereBoundShaderState[EAtmosphereRenderFlag::E_RenderFlagMax];

void SetAtmosphericFogShaders(FRHICommandList& RHICmdList, FScene* Scene, const FViewInfo& View, TRefCountPtr<IPooledRenderTarget>& LightShaftOcclusion)
{
	TShaderMapRef<FAtmosphericVS> VertexShader(GetGlobalShaderMap());
	FAtmosphericFogPS* PixelShader = NULL;
	switch (Scene->AtmosphericFog->RenderFlag)
	{
	default:
	case EAtmosphereRenderFlag::E_EnableAll:
		PixelShader = *TShaderMapRef<TAtmosphericFogPS<EAtmosphereRenderFlag::E_EnableAll> >(GetGlobalShaderMap());
		break;
	case EAtmosphereRenderFlag::E_DisableSunDisk:
		PixelShader = *TShaderMapRef<TAtmosphericFogPS<EAtmosphereRenderFlag::E_DisableSunDisk> >(GetGlobalShaderMap());
		break;
	case EAtmosphereRenderFlag::E_DisableGroundScattering:
		PixelShader = *TShaderMapRef<TAtmosphericFogPS<EAtmosphereRenderFlag::E_DisableGroundScattering> >(GetGlobalShaderMap());
		break;
	case EAtmosphereRenderFlag::E_DisableSunAndGround :
		PixelShader = *TShaderMapRef<TAtmosphericFogPS<EAtmosphereRenderFlag::E_DisableSunAndGround> >(GetGlobalShaderMap());
		break;
	case EAtmosphereRenderFlag::E_DisableLightShaft :
		PixelShader = *TShaderMapRef<TAtmosphericFogPS<EAtmosphereRenderFlag::E_DisableLightShaft> >(GetGlobalShaderMap());
		break;
	case EAtmosphereRenderFlag::E_DisableSunAndLightShaft :
		PixelShader = *TShaderMapRef<TAtmosphericFogPS<EAtmosphereRenderFlag::E_DisableSunAndLightShaft> >(GetGlobalShaderMap());
		break;
	case EAtmosphereRenderFlag::E_DisableGroundAndLightShaft :
		PixelShader = *TShaderMapRef<TAtmosphericFogPS<EAtmosphereRenderFlag::E_DisableGroundAndLightShaft> >(GetGlobalShaderMap());
		break;
	case EAtmosphereRenderFlag::E_DisableAll:
		PixelShader = *TShaderMapRef<TAtmosphericFogPS<EAtmosphereRenderFlag::E_DisableAll> >(GetGlobalShaderMap());
		break;
	}
	
	SetGlobalBoundShaderState(RHICmdList, AtmosphereBoundShaderState[Scene->AtmosphericFog->RenderFlag], GAtmophereVertexDeclaration.VertexDeclarationRHI, *VertexShader, PixelShader);
	VertexShader->SetParameters(RHICmdList, View);
	PixelShader->SetParameters(RHICmdList, View, LightShaftOcclusion);
}

void FDeferredShadingSceneRenderer::RenderAtmosphere(FRHICommandListImmediate& RHICmdList, FLightShaftsOutput LightShaftsOutput)
{
	// Atmospheric fog?
	if (Scene->GetFeatureLevel() >= ERHIFeatureLevel::SM4 && Scene->HasAtmosphericFog())
	{
		SCOPED_DRAW_EVENT(Fog, DEC_SCENE_ITEMS);

		static const FVector2D Vertices[4] =
		{
			FVector2D(-1,-1),
			FVector2D(-1,+1),
			FVector2D(+1,+1),
			FVector2D(+1,-1),
		};
		static const uint16 Indices[6] =
		{
			0, 1, 2,
			0, 2, 3
		};

		
		GSceneRenderTargets.BeginRenderingSceneColor(RHICmdList);
		for(int32 ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			const FViewInfo& View = Views[ViewIndex];

			// Set the device viewport for the view.
			RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0.0f, View.ViewRect.Max.X, View.ViewRect.Max.Y, 1.0f);
			
			RHICmdList.SetRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::GetRHI());
			
			// disable alpha writes in order to preserve scene depth values on PC
			RHICmdList.SetBlendState(TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_SourceAlpha>::GetRHI());

			RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false,CF_Always>::GetRHI());

			SetAtmosphericFogShaders(RHICmdList, Scene, View, LightShaftsOutput.LightShaftOcclusion);

			// Draw a quad covering the view.
			DrawIndexedPrimitiveUP(
				RHICmdList,
				PT_TriangleList,
				0,
				ARRAY_COUNT(Vertices),
				2,
				Indices,
				sizeof(Indices[0]),
				Vertices,
				sizeof(Vertices[0])
				);
		}

		//no need to resolve since we used alpha blending
		GSceneRenderTargets.FinishRenderingSceneColor(RHICmdList, false);
	}
}

#if WITH_EDITOR
class FAtmosphereTransmittancePS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereTransmittancePS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereTransmittancePS() {}

public:
	/* Initialization constructor. */
	FAtmosphereTransmittancePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, const FViewInfo& View)
	{
		FGlobalShader::SetParameters(RHICmdList, GetPixelShader(), View);
	}
};

class FAtmosphereIrradiance1PS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereIrradiance1PS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereIrradiance1PS() {}

public:
	FAtmosphereShaderPrecomputeTextureParameters AtmosphereParameters;

	/* Initialization constructor. */
	FAtmosphereIrradiance1PS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereParameters.Bind(Initializer.ParameterMap, 0, FAtmosphereShaderPrecomputeTextureParameters::Transmittance);
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereParameters;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, const FAtmosphereTextures* Textures)
	{
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 0, FAtmosphereShaderPrecomputeTextureParameters::Transmittance, Textures);
	}
};

class FAtmosphereIrradianceNPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereIrradianceNPS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereIrradianceNPS() {}

public:
	FAtmosphereShaderPrecomputeTextureParameters AtmosphereParameters;
	FShaderParameter FirstOrderParameter;

	/* Initialization constructor. */
	FAtmosphereIrradianceNPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereParameters.Bind(Initializer.ParameterMap, 0, FAtmosphereShaderPrecomputeTextureParameters::Transmittance);
		AtmosphereParameters.Bind(Initializer.ParameterMap, 1, FAtmosphereShaderPrecomputeTextureParameters::DeltaSR);
		AtmosphereParameters.Bind(Initializer.ParameterMap, 2, FAtmosphereShaderPrecomputeTextureParameters::DeltaSM);
		FirstOrderParameter.Bind(Initializer.ParameterMap, TEXT("FirstOrder"));
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereParameters << FirstOrderParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, const FViewInfo& View, float FirstOrder, const FAtmosphereTextures* Textures)
	{
		FGlobalShader::SetParameters(RHICmdList, GetPixelShader(), View);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 0, FAtmosphereShaderPrecomputeTextureParameters::Transmittance, Textures);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 1, FAtmosphereShaderPrecomputeTextureParameters::DeltaSR, Textures);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 2, FAtmosphereShaderPrecomputeTextureParameters::DeltaSM, Textures);
		SetShaderValue(RHICmdList, GetPixelShader(), FirstOrderParameter, FirstOrder);
	}
};

class FAtmosphereCopyIrradiancePS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereCopyIrradiancePS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereCopyIrradiancePS() {}

public:
	FAtmosphereShaderPrecomputeTextureParameters AtmosphereParameters;

	/* Initialization constructor. */
	FAtmosphereCopyIrradiancePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereParameters.Bind(Initializer.ParameterMap, 0, FAtmosphereShaderPrecomputeTextureParameters::DeltaE);
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereParameters;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, const FAtmosphereTextures* Textures)
	{
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 0, FAtmosphereShaderPrecomputeTextureParameters::DeltaE, Textures);
	}
};

class FAtmosphereGS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereGS,Global);

	static bool ShouldCache(EShaderPlatform Platform) 
	{ 
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4); 
	}

	FAtmosphereGS() {}

public:
	FShaderParameter AtmosphereLayerParameter;

	FAtmosphereGS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereLayerParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereLayer"));
	}

	void SetParameters(FRHICommandList& RHICmdList, int AtmosphereLayer)
	{
		SetShaderValue(RHICmdList, GetGeometryShader(), AtmosphereLayerParameter, AtmosphereLayer);
	}

	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereLayerParameter;
		return bShaderHasOutdatedParameters;
	}
};

class FAtmosphereInscatter1PS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereInscatter1PS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereInscatter1PS() {}

public:
	FAtmosphereShaderPrecomputeTextureParameters AtmosphereParameters;
	FShaderParameter dhdHParameter;
	FShaderParameter AtmosphereRParameter;

	/* Initialization constructor. */
	FAtmosphereInscatter1PS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereParameters.Bind(Initializer.ParameterMap, 0, FAtmosphereShaderPrecomputeTextureParameters::Transmittance);
		dhdHParameter.Bind(Initializer.ParameterMap, TEXT("DhdH"));
		AtmosphereRParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereR"));
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereParameters << dhdHParameter << AtmosphereRParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, const FViewInfo& View, float AtmosphereR, const FVector4& DhdH, const FAtmosphereTextures* Textures)
	{
		FGlobalShader::SetParameters(RHICmdList, GetPixelShader(), View);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 0, FAtmosphereShaderPrecomputeTextureParameters::Transmittance, Textures);
		SetShaderValue(RHICmdList, GetPixelShader(), dhdHParameter, DhdH);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereRParameter, AtmosphereR);
	}
};

class FAtmosphereCopyInscatter1PS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereCopyInscatter1PS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereCopyInscatter1PS() {}

public:
	FAtmosphereShaderPrecomputeTextureParameters AtmosphereParameters;
	FShaderParameter dhdHParameter;
	FShaderParameter AtmosphereRParameter;
	FShaderParameter AtmosphereLayerParameter;

	/* Initialization constructor. */
	FAtmosphereCopyInscatter1PS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereParameters.Bind(Initializer.ParameterMap, 0, FAtmosphereShaderPrecomputeTextureParameters::DeltaSR);
		AtmosphereParameters.Bind(Initializer.ParameterMap, 1, FAtmosphereShaderPrecomputeTextureParameters::DeltaSM);
		dhdHParameter.Bind(Initializer.ParameterMap, TEXT("DhdH"));
		AtmosphereRParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereR"));
		AtmosphereLayerParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereLayer"));
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereParameters << dhdHParameter << AtmosphereRParameter << AtmosphereLayerParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, const FViewInfo& View, float AtmosphereR, const FVector4& DhdH, int AtmosphereLayer, const FAtmosphereTextures* Textures)
	{
		FGlobalShader::SetParameters(RHICmdList, GetPixelShader(), View);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 0, FAtmosphereShaderPrecomputeTextureParameters::DeltaSR, Textures);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 1, FAtmosphereShaderPrecomputeTextureParameters::DeltaSM, Textures);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereRParameter, AtmosphereR);
		SetShaderValue(RHICmdList, GetPixelShader(), dhdHParameter, DhdH);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereLayerParameter, AtmosphereLayer);
	}
};

class FAtmosphereCopyInscatterNPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereCopyInscatterNPS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereCopyInscatterNPS() {}

public:
	FAtmosphereShaderPrecomputeTextureParameters AtmosphereParameters;
	FShaderParameter dhdHParameter;
	FShaderParameter AtmosphereRParameter;
	FShaderParameter AtmosphereLayerParameter;

	/* Initialization constructor. */
	FAtmosphereCopyInscatterNPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereParameters.Bind(Initializer.ParameterMap, 0, FAtmosphereShaderPrecomputeTextureParameters::DeltaSR);
		dhdHParameter.Bind(Initializer.ParameterMap, TEXT("DhdH"));
		AtmosphereRParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereR"));
		AtmosphereLayerParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereLayer"));
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereParameters << dhdHParameter << AtmosphereRParameter << AtmosphereLayerParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, const FViewInfo& View, float AtmosphereR, const FVector4& DhdH, int AtmosphereLayer, const FAtmosphereTextures* Textures)
	{
		FGlobalShader::SetParameters(RHICmdList, GetPixelShader(), View);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 0, FAtmosphereShaderPrecomputeTextureParameters::DeltaSR, Textures);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereRParameter, AtmosphereR);
		SetShaderValue(RHICmdList, GetPixelShader(), dhdHParameter, DhdH);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereLayerParameter, AtmosphereLayer);
	}
};

class FAtmosphereInscatterSPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereInscatterSPS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereInscatterSPS() {}

public:
	FAtmosphereShaderPrecomputeTextureParameters AtmosphereParameters;
	FShaderParameter dhdHParameter;
	FShaderParameter AtmosphereRParameter;
	FShaderParameter FirstOrderParameter;

	/* Initialization constructor. */
	FAtmosphereInscatterSPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereParameters.Bind(Initializer.ParameterMap, 0, FAtmosphereShaderPrecomputeTextureParameters::Transmittance);
		AtmosphereParameters.Bind(Initializer.ParameterMap, 1, FAtmosphereShaderPrecomputeTextureParameters::DeltaE);
		AtmosphereParameters.Bind(Initializer.ParameterMap, 2, FAtmosphereShaderPrecomputeTextureParameters::DeltaSR);
		AtmosphereParameters.Bind(Initializer.ParameterMap, 3, FAtmosphereShaderPrecomputeTextureParameters::DeltaSM);
		dhdHParameter.Bind(Initializer.ParameterMap, TEXT("DhdH"));
		AtmosphereRParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereR"));
		FirstOrderParameter.Bind(Initializer.ParameterMap, TEXT("FirstOrder"));
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereParameters << dhdHParameter << AtmosphereRParameter << FirstOrderParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, const FViewInfo& View, float AtmosphereR, const FVector4& DhdH, float FirstOrder, const FAtmosphereTextures* Textures)
	{
		FGlobalShader::SetParameters(RHICmdList, GetPixelShader(), View);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 0, FAtmosphereShaderPrecomputeTextureParameters::Transmittance, Textures);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 1, FAtmosphereShaderPrecomputeTextureParameters::DeltaE, Textures);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 2, FAtmosphereShaderPrecomputeTextureParameters::DeltaSR, Textures);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 3, FAtmosphereShaderPrecomputeTextureParameters::DeltaSM, Textures);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereRParameter, AtmosphereR);
		SetShaderValue(RHICmdList, GetPixelShader(), dhdHParameter, DhdH);
		SetShaderValue(RHICmdList, GetPixelShader(), FirstOrderParameter, FirstOrder);
	}
};

class FAtmosphereInscatterNPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereInscatterNPS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereInscatterNPS() {}

public:
	FAtmosphereShaderPrecomputeTextureParameters AtmosphereParameters;
	FShaderParameter dhdHParameter;
	FShaderParameter AtmosphereRParameter;
	FShaderParameter FirstOrderParameter;

	/* Initialization constructor. */
	FAtmosphereInscatterNPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereParameters.Bind(Initializer.ParameterMap, 0, FAtmosphereShaderPrecomputeTextureParameters::Transmittance);
		AtmosphereParameters.Bind(Initializer.ParameterMap, 1, FAtmosphereShaderPrecomputeTextureParameters::DeltaJ);
		dhdHParameter.Bind(Initializer.ParameterMap, TEXT("DhdH"));
		AtmosphereRParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereR"));
		FirstOrderParameter.Bind(Initializer.ParameterMap, TEXT("FirstOrder"));
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereParameters << dhdHParameter << AtmosphereRParameter << FirstOrderParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, const FViewInfo& View, float AtmosphereR, const FVector4& DhdH, float FirstOrder, const FAtmosphereTextures* Textures)
	{
		FGlobalShader::SetParameters(RHICmdList, GetPixelShader(), View);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 0, FAtmosphereShaderPrecomputeTextureParameters::Transmittance, Textures);
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 1, FAtmosphereShaderPrecomputeTextureParameters::DeltaJ, Textures);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereRParameter, AtmosphereR);
		SetShaderValue(RHICmdList, GetPixelShader(), dhdHParameter, DhdH);
		SetShaderValue(RHICmdList, GetPixelShader(), FirstOrderParameter, FirstOrder);
	}
};

class FAtmospherePrecomputeVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmospherePrecomputeVS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmospherePrecomputeVS() {}

public:

	/* Initialization constructor. */
	FAtmospherePrecomputeVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
	}

	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		return bShaderHasOutdatedParameters;
	}
};

// Final Fix
class FAtmosphereCopyInscatterFPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereCopyInscatterFPS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereCopyInscatterFPS() {}

public:
	FAtmosphereShaderPrecomputeTextureParameters AtmosphereParameters;
	FShaderParameter dhdHParameter;
	FShaderParameter AtmosphereRParameter;
	FShaderParameter AtmosphereLayerParameter;

	/* Initialization constructor. */
	FAtmosphereCopyInscatterFPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereParameters.Bind(Initializer.ParameterMap, 0, FAtmosphereShaderPrecomputeTextureParameters::Inscatter);
		dhdHParameter.Bind(Initializer.ParameterMap, TEXT("DhdH"));
		AtmosphereRParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereR"));
		AtmosphereLayerParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereLayer"));
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereParameters << dhdHParameter << AtmosphereRParameter << AtmosphereLayerParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, float AtmosphereR, const FVector4& DhdH, int AtmosphereLayer, const FAtmosphereTextures* Textures)
	{
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 0, FAtmosphereShaderPrecomputeTextureParameters::Inscatter, Textures);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereRParameter, AtmosphereR);
		SetShaderValue(RHICmdList, GetPixelShader(), dhdHParameter, DhdH);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereLayerParameter, AtmosphereLayer);
	}
};

class FAtmosphereCopyInscatterFBackPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAtmosphereCopyInscatterFBackPS,Global);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4);
	}

	/* Default constructor. */
	FAtmosphereCopyInscatterFBackPS() {}

public:
	FAtmosphereShaderPrecomputeTextureParameters AtmosphereParameters;
	FShaderParameter dhdHParameter;
	FShaderParameter AtmosphereRParameter;
	FShaderParameter AtmosphereLayerParameter;

	/* Initialization constructor. */
	FAtmosphereCopyInscatterFBackPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		AtmosphereParameters.Bind(Initializer.ParameterMap, 0, FAtmosphereShaderPrecomputeTextureParameters::DeltaSR);
		dhdHParameter.Bind(Initializer.ParameterMap, TEXT("DhdH"));
		AtmosphereRParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereR"));
		AtmosphereLayerParameter.Bind(Initializer.ParameterMap, TEXT("AtmosphereLayer"));
	}

	// FShader interface.
	virtual bool Serialize(FArchive& Ar)
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << AtmosphereParameters << dhdHParameter << AtmosphereRParameter << AtmosphereLayerParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandList& RHICmdList, float AtmosphereR, const FVector4& DhdH, int AtmosphereLayer, const FAtmosphereTextures* Textures)
	{
		AtmosphereParameters.Set(RHICmdList, GetPixelShader(), 0, FAtmosphereShaderPrecomputeTextureParameters::DeltaSR, Textures);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereRParameter, AtmosphereR);
		SetShaderValue(RHICmdList, GetPixelShader(), dhdHParameter, DhdH);
		SetShaderValue(RHICmdList, GetPixelShader(), AtmosphereLayerParameter, AtmosphereLayer);
	}
};

IMPLEMENT_SHADER_TYPE(,FAtmosphereTransmittancePS,TEXT("AtmospherePrecompute"),TEXT("TransmittancePS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmosphereIrradiance1PS,TEXT("AtmospherePrecompute"),TEXT("Irradiance1PS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmosphereIrradianceNPS,TEXT("AtmospherePrecompute"),TEXT("IrradianceNPS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmosphereCopyIrradiancePS,TEXT("AtmospherePrecompute"),TEXT("CopyIrradiancePS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmosphereGS,TEXT("AtmospherePrecomputeInscatter"),TEXT("AtmosphereGS"),SF_Geometry);
IMPLEMENT_SHADER_TYPE(,FAtmosphereInscatter1PS,TEXT("AtmospherePrecomputeInscatter"),TEXT("Inscatter1PS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmosphereCopyInscatter1PS,TEXT("AtmospherePrecomputeInscatter"),TEXT("CopyInscatter1PS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmosphereCopyInscatterNPS,TEXT("AtmospherePrecomputeInscatter"),TEXT("CopyInscatterNPS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmosphereInscatterSPS,TEXT("AtmospherePrecomputeInscatter"),TEXT("InscatterSPS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmosphereInscatterNPS,TEXT("AtmospherePrecomputeInscatter"),TEXT("InscatterNPS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmosphereCopyInscatterFPS,TEXT("AtmospherePrecomputeInscatter"),TEXT("CopyInscatterFPS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmosphereCopyInscatterFBackPS,TEXT("AtmospherePrecomputeInscatter"),TEXT("CopyInscatterFBackPS"),SF_Pixel);
IMPLEMENT_SHADER_TYPE(,FAtmospherePrecomputeVS,TEXT("AtmospherePrecompute"),TEXT("MainVS"),SF_Vertex);

namespace
{
	const float RadiusGround = 6360;
	const float RadiusAtmosphere = 6420;

	enum
	{
		AP_Transmittance = 0,
		AP_Irradiance1,
		AP_Inscatter1,
		AP_ClearIrradiance,
		AP_CopyInscatter1,
		AP_StartOrder,
		AP_InscatterS,
		AP_IrradianceN,
		AP_InscatterN,
		AP_CopyIrradiance,
		AP_CopyInscatterN,
		AP_EndOrder,
		AP_CopyInscatterF,
		AP_CopyInscatterFBack,
		AP_MAX
	};
}

void FAtmosphericFogSceneInfo::StartPrecompute()
{
	bNeedRecompute = false;
	AtmospherePhase = 0;
	Atmosphere3DTextureIndex = 0;
	AtmoshpereOrder = 2;
}

FIntPoint FAtmosphericFogSceneInfo::GetTextureSize()
{
	switch(AtmospherePhase)
	{
	case AP_Transmittance:
		return AtmosphereTextures->AtmosphereTransmittance->GetDesc().Extent;
	case AP_ClearIrradiance:
	case AP_CopyIrradiance:
	case AP_Irradiance1:
	case AP_IrradianceN:
		return AtmosphereTextures->AtmosphereIrradiance->GetDesc().Extent;
	case AP_Inscatter1:
	case AP_CopyInscatter1:
	case AP_CopyInscatterF:
	case AP_CopyInscatterFBack:
	case AP_InscatterN:
	case AP_CopyInscatterN:
	case AP_InscatterS:
		return AtmosphereTextures->AtmosphereInscatter->GetDesc().Extent;
	}
	return AtmosphereTextures->AtmosphereTransmittance->GetDesc().Extent;
}

void FAtmosphericFogSceneInfo::DrawQuad(FRHICommandList& RHICmdList, const FIntRect& ViewRect, FShader* VertexShader)
{
	// Draw a quad mapping scene color to the view's render target
	DrawRectangle( 
		RHICmdList,
		ViewRect.Min.X, ViewRect.Min.Y, 
		ViewRect.Width(), ViewRect.Height(),
		ViewRect.Min.X, ViewRect.Min.Y, 
		ViewRect.Width(), ViewRect.Height(),
		ViewRect.Size(),
		ViewRect.Size(),
		VertexShader,
		EDRF_UseTriangleOptimization);
}

void FAtmosphericFogSceneInfo::GetLayerValue(int Layer, float& AtmosphereR, FVector4& DhdH)
{
	float R = Layer / FMath::Max<float>(Component->PrecomputeParams.InscatterAltitudeSampleNum - 1.f, 1.f);
	R = R * R;
	R = sqrt(RadiusGround * RadiusGround + R * (RadiusAtmosphere * RadiusAtmosphere - RadiusGround * RadiusGround)) + (Layer == 0 ? 0.01 : (Layer == Component->PrecomputeParams.InscatterAltitudeSampleNum - 1 ? -0.001 : 0.0));
	float DMin = RadiusAtmosphere - R;
	float DMax = sqrt(R * R - RadiusGround * RadiusGround) + sqrt(RadiusAtmosphere * RadiusAtmosphere - RadiusGround * RadiusGround);
	float DMinP = R - RadiusGround;
	float DMaxP = sqrt(R * R - RadiusGround * RadiusGround);
	AtmosphereR = R;
	DhdH = FVector4(DMin, DMax, DMinP, DMaxP);
}

void FAtmosphericFogSceneInfo::RenderAtmosphereShaders(FRHICommandListImmediate& RHICmdList, const FViewInfo& View, const FIntRect& ViewRect)
{
	check(Component != NULL);
	switch(AtmospherePhase)
	{
	case AP_Transmittance:
		{
			const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereTransmittance->GetRenderTargetItem();
			SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

			TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
			TShaderMapRef<FAtmosphereTransmittancePS> PixelShader(GetGlobalShaderMap());

			static FGlobalBoundShaderState BoundShaderState;
			SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader);
			PixelShader->SetParameters(RHICmdList, View);
			DrawQuad(RHICmdList, ViewRect, *VertexShader);

			RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());
		}
		break;
	case AP_Irradiance1:
		{
			const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereDeltaE->GetRenderTargetItem();
			SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

			TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
			TShaderMapRef<FAtmosphereIrradiance1PS> PixelShader(GetGlobalShaderMap());

			static FGlobalBoundShaderState BoundShaderState;
			SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader);

			PixelShader->SetParameters(RHICmdList, AtmosphereTextures);

			DrawQuad(RHICmdList, ViewRect, *VertexShader);

			RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());
		}
		break;
	case AP_Inscatter1:
		{
			int32 Layer = Atmosphere3DTextureIndex;
			{
				FTextureRHIParamRef RenderTargets[2];
				RenderTargets[0] = AtmosphereTextures->AtmosphereDeltaSR->GetRenderTargetItem().TargetableTexture;
				RenderTargets[1] = AtmosphereTextures->AtmosphereDeltaSM->GetRenderTargetItem().TargetableTexture;
				SetRenderTargets(RHICmdList, ARRAY_COUNT(RenderTargets), RenderTargets, FTextureRHIRef(), 0, NULL);

				TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereGS> GeometryShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereInscatter1PS> PixelShader(GetGlobalShaderMap());

				static FGlobalBoundShaderState BoundShaderState;
				
				SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, *GeometryShader);

				//
				float r;
				FVector4 DhdH;
				GetLayerValue(Layer, r, DhdH);
				GeometryShader->SetParameters(RHICmdList, Layer);
				PixelShader->SetParameters(RHICmdList, View, r, DhdH, AtmosphereTextures);
				DrawQuad(RHICmdList, ViewRect, *VertexShader);

				if (Atmosphere3DTextureIndex == Component->PrecomputeParams.InscatterAltitudeSampleNum - 1)
				{
					RHICmdList.CopyToResolveTarget(AtmosphereTextures->AtmosphereDeltaSR->GetRenderTargetItem().TargetableTexture,
						AtmosphereTextures->AtmosphereDeltaSR->GetRenderTargetItem().ShaderResourceTexture, true, FResolveParams());
					RHICmdList.CopyToResolveTarget(AtmosphereTextures->AtmosphereDeltaSM->GetRenderTargetItem().TargetableTexture,
						AtmosphereTextures->AtmosphereDeltaSM->GetRenderTargetItem().ShaderResourceTexture, true, FResolveParams());
				}
			}
		}
		break;
	case AP_ClearIrradiance:
		{
			const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereIrradiance->GetRenderTargetItem();
			SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

			RHICmdList.Clear(true, FLinearColor::Black, false, 0.0f, false, 0, FIntRect());
			RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());
		}
		break;
	case AP_CopyInscatter1:
		{
			int32 Layer = Atmosphere3DTextureIndex;
			{
				const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereInscatter->GetRenderTargetItem();
				SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

				TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereGS> GeometryShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereCopyInscatter1PS> PixelShader(GetGlobalShaderMap());

				static FGlobalBoundShaderState BoundShaderState;
				
				SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, *GeometryShader);

				float r;
				FVector4 DhdH;
				GetLayerValue(Layer, r, DhdH);
				GeometryShader->SetParameters(RHICmdList, Layer);
				PixelShader->SetParameters(RHICmdList, View, r, DhdH, Layer, AtmosphereTextures);
				DrawQuad(RHICmdList, ViewRect, *VertexShader);

				if (Atmosphere3DTextureIndex == Component->PrecomputeParams.InscatterAltitudeSampleNum - 1)
				{
					RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());
				}
			}
		}
		break;
	case AP_InscatterS:
		{
			int32 Layer = Atmosphere3DTextureIndex;
			{
				const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereDeltaJ->GetRenderTargetItem();
				SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

				TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereGS> GeometryShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereInscatterSPS> PixelShader(GetGlobalShaderMap());

				static FGlobalBoundShaderState BoundShaderState;
				
				SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, *GeometryShader);

				float r;
				FVector4 DhdH;
				GetLayerValue(Layer, r, DhdH);
				GeometryShader->SetParameters(RHICmdList, Layer);
				PixelShader->SetParameters(RHICmdList, View, r, DhdH, AtmoshpereOrder == 2 ? 1.f : 0.f, AtmosphereTextures);
				DrawQuad(RHICmdList, ViewRect, *VertexShader);

				if (Atmosphere3DTextureIndex == Component->PrecomputeParams.InscatterAltitudeSampleNum - 1)
				{
					RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());
				}
			}
		}
		break;
	case AP_IrradianceN:
		{
			const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereDeltaE->GetRenderTargetItem();
			SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

			TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
			TShaderMapRef<FAtmosphereIrradianceNPS> PixelShader(GetGlobalShaderMap());

			static FGlobalBoundShaderState BoundShaderState;
			
			SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader);

			PixelShader->SetParameters(RHICmdList, View, AtmoshpereOrder == 2 ? 1.f : 0.f, AtmosphereTextures);

			DrawQuad(RHICmdList, ViewRect, *VertexShader);

			RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());
		}
		break;

	case AP_InscatterN:
		{
			int32 Layer = Atmosphere3DTextureIndex;
			{
				const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereDeltaSR->GetRenderTargetItem();
				SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

				TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereGS> GeometryShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereInscatterNPS> PixelShader(GetGlobalShaderMap());

				static FGlobalBoundShaderState BoundShaderState;
				
				SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, *GeometryShader);

				float r;
				FVector4 DhdH;
				GetLayerValue(Layer, r, DhdH);
				GeometryShader->SetParameters(RHICmdList, Layer);
				PixelShader->SetParameters(RHICmdList, View, r, DhdH, AtmoshpereOrder == 2 ? 1.f : 0.f, AtmosphereTextures);
				DrawQuad(RHICmdList, ViewRect, *VertexShader);

				if (Atmosphere3DTextureIndex == Component->PrecomputeParams.InscatterAltitudeSampleNum - 1)
				{
					RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());
				}
			}
		}
		break;
	case AP_CopyIrradiance:
		{
			RHICmdList.SetBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::GetRHI());

			const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereIrradiance->GetRenderTargetItem();
			SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

			TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
			TShaderMapRef<FAtmosphereCopyIrradiancePS> PixelShader(GetGlobalShaderMap());

			static FGlobalBoundShaderState BoundShaderState;
			
			SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader);

			PixelShader->SetParameters(RHICmdList, AtmosphereTextures);

			DrawQuad(RHICmdList, ViewRect, *VertexShader);

			RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());

			RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
		}
		break;

	case AP_CopyInscatterN:
		{
			RHICmdList.SetBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::GetRHI());

			int32 Layer = Atmosphere3DTextureIndex;
			{
				const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereInscatter->GetRenderTargetItem();
				SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

				TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereGS> GeometryShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereCopyInscatterNPS> PixelShader(GetGlobalShaderMap());

				static FGlobalBoundShaderState BoundShaderState;
				
				SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, *GeometryShader);

				float r;
				FVector4 DhdH;
				GetLayerValue(Layer, r, DhdH);
				GeometryShader->SetParameters(RHICmdList, Layer);
				PixelShader->SetParameters(RHICmdList, View, r, DhdH, Layer, AtmosphereTextures);
				DrawQuad(RHICmdList, ViewRect, *VertexShader);

				if (Atmosphere3DTextureIndex == Component->PrecomputeParams.InscatterAltitudeSampleNum - 1)
				{
					RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());
				}
			}

			RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
		}
		break;

	case AP_CopyInscatterF:
		{
			int32 Layer = Atmosphere3DTextureIndex;
			{
				const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereDeltaSR->GetRenderTargetItem();
				SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

				TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereGS> GeometryShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereCopyInscatterFPS> PixelShader(GetGlobalShaderMap());

				static FGlobalBoundShaderState BoundShaderState;
				
				SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, *GeometryShader);

				float r;
				FVector4 DhdH;
				GetLayerValue(Layer, r, DhdH);
				GeometryShader->SetParameters(RHICmdList, Layer);
				PixelShader->SetParameters(RHICmdList, r, DhdH, Layer, AtmosphereTextures);
				DrawQuad(RHICmdList, ViewRect, *VertexShader);

				if (Atmosphere3DTextureIndex == Component->PrecomputeParams.InscatterAltitudeSampleNum - 1)
				{
					RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());
				}
			}
		}
		break;
	case AP_CopyInscatterFBack:
		{
			int32 Layer = Atmosphere3DTextureIndex;
			{
				const FSceneRenderTargetItem& DestRenderTarget = AtmosphereTextures->AtmosphereInscatter->GetRenderTargetItem();
				SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());

				TShaderMapRef<FAtmospherePrecomputeVS> VertexShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereGS> GeometryShader(GetGlobalShaderMap());
				TShaderMapRef<FAtmosphereCopyInscatterFBackPS> PixelShader(GetGlobalShaderMap());

				static FGlobalBoundShaderState BoundShaderState;
				
				SetGlobalBoundShaderState(RHICmdList, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, *GeometryShader);

				float r;
				FVector4 DhdH;
				GetLayerValue(Layer, r, DhdH);
				GeometryShader->SetParameters(RHICmdList, Layer);
				PixelShader->SetParameters(RHICmdList, r, DhdH, Layer, AtmosphereTextures);
				DrawQuad(RHICmdList, ViewRect, *VertexShader);

				if (Atmosphere3DTextureIndex == Component->PrecomputeParams.InscatterAltitudeSampleNum - 1)
				{
					RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, true, FResolveParams());
				}
			}
		}
		break;

	}
}

void FAtmosphericFogSceneInfo::PrecomputeAtmosphereData(FRHICommandListImmediate& RHICmdList, const FViewInfo* View, FSceneViewFamily& ViewFamily)
{
	// Set the view family's render target/viewport.
	FIntPoint TexSize = GetTextureSize();
	FIntRect ViewRect(0, 0, TexSize.X, TexSize.Y);

	// turn off culling and blending
	RHICmdList.SetRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::GetRHI());
	RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
	// turn off depth reads/writes
	RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());

	RHICmdList.SetViewport(0, 0, 0.0f, TexSize.X, TexSize.Y, 0.0f);

	RenderAtmosphereShaders(RHICmdList, *View, ViewRect);
}

void FAtmosphericFogSceneInfo::ReadPixelsPtr(FRHICommandListImmediate& RHICmdList, TRefCountPtr<IPooledRenderTarget> RenderTarget, FColor* OutData, FIntRect InRect)
{
	TArray<FFloat16Color> Data;

	RHICmdList.ReadSurfaceFloatData(
		RenderTarget->GetRenderTargetItem().ShaderResourceTexture,
		InRect,
		Data,
		CubeFace_PosX,
		0,
		0
		);

	// Convert from FFloat16Color to FColor
	for (int32 i = 0; i < Data.Num(); ++i)
	{
		FColor TempColor;
		TempColor.R = FMath::Clamp<uint8>(Data[i].R.GetFloat() * 255, 0, 255);
		TempColor.G = FMath::Clamp<uint8>(Data[i].G.GetFloat() * 255, 0, 255);
		TempColor.B = FMath::Clamp<uint8>(Data[i].B.GetFloat() * 255, 0, 255);
		OutData[i] = TempColor;
	}
}

void FAtmosphericFogSceneInfo::Read3DPixelsPtr(FRHICommandListImmediate& RHICmdList, TRefCountPtr<IPooledRenderTarget> RenderTarget, FFloat16Color* OutData, FIntRect InRect, FIntPoint InZMinMax)
{
	TArray<FFloat16Color> Data;

	RHICmdList.Read3DSurfaceFloatData(
		RenderTarget->GetRenderTargetItem().ShaderResourceTexture,
		InRect,
		InZMinMax,
		Data
		);

	FMemory::Memcpy( OutData, Data.GetTypedData(), Data.Num() * sizeof(FFloat16Color) );
}

void FAtmosphericFogSceneInfo::PrecomputeTextures(FRHICommandListImmediate& RHICmdList, const FViewInfo* View, FSceneViewFamily* ViewFamily)
{
	check(Component != NULL);
	if (AtmosphereTextures == NULL)
	{
		AtmosphereTextures = new FAtmosphereTextures(&Component->PrecomputeParams);
	}

	if (bNeedRecompute)
	{
		StartPrecompute();
	}

	// Atmosphere 
	if (PrecomputeCounter.GetValue() < UAtmosphericFogComponent::EFinishedComputation)
	{
		PrecomputeAtmosphereData(RHICmdList, View, *ViewFamily);

		switch(AtmospherePhase)
		{
		case AP_Inscatter1:
		case AP_CopyInscatter1:
		case AP_CopyInscatterF:
		case AP_CopyInscatterFBack:
		case AP_InscatterN:
		case AP_CopyInscatterN:
		case AP_InscatterS:
			Atmosphere3DTextureIndex++;
			if (Atmosphere3DTextureIndex >= Component->PrecomputeParams.InscatterAltitudeSampleNum)
			{
				AtmospherePhase++;
				Atmosphere3DTextureIndex = 0;
			}
			break;
		default:
			AtmospherePhase++;
			break;
		}

		if (AtmospherePhase == AP_EndOrder)
		{
			AtmospherePhase = AP_StartOrder;
			AtmoshpereOrder++;
		}

		if (AtmospherePhase == AP_StartOrder)
		{
			if (AtmoshpereOrder > MaxScatteringOrder)
			{
				if (Component->PrecomputeParams.DensityHeight > 0.678f) // Fixed artifacts only for some value
				{
					AtmospherePhase = AP_CopyInscatterF;
				}
				else
				{
					AtmospherePhase = AP_MAX;
				}
				AtmoshpereOrder = 2;
			}
		}

		if (AtmospherePhase >= AP_MAX)
		{
			AtmospherePhase = 0;
			Atmosphere3DTextureIndex = 0;
			AtmoshpereOrder = 2;

			// Save precomputed data to bulk data
			{
				FIntPoint Extent = AtmosphereTextures->AtmosphereTransmittance->GetDesc().Extent;
				int32 TotalByte = sizeof(FColor) * Extent.X * Extent.Y;
				PrecomputeTransmittance.Lock(LOCK_READ_WRITE);
				FColor* TransmittanceData = (FColor*)PrecomputeTransmittance.Realloc(TotalByte);
				ReadPixelsPtr(RHICmdList, AtmosphereTextures->AtmosphereTransmittance, TransmittanceData, FIntRect(0, 0, Extent.X, Extent.Y));
				PrecomputeTransmittance.Unlock();
			}

			{
				FIntPoint Extent = AtmosphereTextures->AtmosphereIrradiance->GetDesc().Extent;
				int32 TotalByte = sizeof(FColor) * Extent.X * Extent.Y;
				PrecomputeIrradiance.Lock(LOCK_READ_WRITE);
				FColor* IrradianceData = (FColor*)PrecomputeIrradiance.Realloc(TotalByte);
				ReadPixelsPtr(RHICmdList, AtmosphereTextures->AtmosphereIrradiance, IrradianceData, FIntRect(0, 0, Extent.X, Extent.Y));
				PrecomputeIrradiance.Unlock();
			}

			{
				int32 SizeX = Component->PrecomputeParams.InscatterMuSNum * Component->PrecomputeParams.InscatterNuNum;
				int32 SizeY = Component->PrecomputeParams.InscatterMuNum;
				int32 SizeZ = Component->PrecomputeParams.InscatterAltitudeSampleNum;
				int32 TotalByte = sizeof(FFloat16Color) * SizeX * SizeY * SizeZ;
				PrecomputeInscatter.Lock(LOCK_READ_WRITE);
				FFloat16Color* InscatterData = (FFloat16Color*)PrecomputeInscatter.Realloc(TotalByte);
				Read3DPixelsPtr(RHICmdList, AtmosphereTextures->AtmosphereInscatter, InscatterData, FIntRect(0, 0, SizeX, SizeY), FIntPoint(0, SizeZ));
				PrecomputeInscatter.Unlock();
			}

			// Delete render targets
			if (AtmosphereTextures)
			{
				delete AtmosphereTextures;
				AtmosphereTextures = NULL;
			}

			// Save to bulk data is done
			PrecomputeCounter.Increment(); // Notice to component that pre-computation is done...
		}
	}
}
#endif

/** Initialization constructor. */
FAtmosphericFogSceneInfo::FAtmosphericFogSceneInfo(UAtmosphericFogComponent* InComponent, const FScene* InScene)
	: Component(InComponent)
	, SunMultiplier(InComponent->SunMultiplier)
	, FogMultiplier(InComponent->FogMultiplier)
	, InvDensityMultiplier(InComponent->DensityMultiplier > 0.f ? 1.f / InComponent->DensityMultiplier : 1.f)
	, DensityOffset(InComponent->DensityOffset)
	, GroundOffset(InComponent->GroundOffset)
	, DistanceScale(InComponent->DistanceScale)
	, AltitudeScale(InComponent->AltitudeScale)
	, RHeight(InComponent->PrecomputeParams.DensityHeight * InComponent->PrecomputeParams.DensityHeight * InComponent->PrecomputeParams.DensityHeight * 64.f)
	, StartDistance(InComponent->StartDistance)
	, DistanceOffset(InComponent->DistanceOffset)
	, SunDiscScale(InComponent->SunDiscScale)
	, RenderFlag(EAtmosphereRenderFlag::E_EnableAll)
	, InscatterAltitudeSampleNum(InComponent->PrecomputeParams.InscatterAltitudeSampleNum)
#if WITH_EDITOR
	, PrecomputeCounter(InComponent->PrecomputeCounter)
	, bNeedRecompute(false)
	, MaxScatteringOrder(InComponent->PrecomputeParams.MaxScatteringOrder)
	, AtmospherePhase(0)
	, Atmosphere3DTextureIndex(0)
	, AtmoshpereOrder(2)
	, AtmosphereTextures(NULL)
#endif
{
	StartDistance *= DistanceScale * 0.00001f; // Convert to km in Atmospheric fog shader
	// DistanceOffset is in km, no need to change...
	DefaultSunColor = FLinearColor(InComponent->DefaultLightColor) * InComponent->DefaultBrightness;
	RenderFlag |= (InComponent->bDisableSunDisk) ? EAtmosphereRenderFlag::E_DisableSunDisk : EAtmosphereRenderFlag::E_EnableAll;
	RenderFlag |= (InComponent->bDisableGroundScattering) ? EAtmosphereRenderFlag::E_DisableGroundScattering : EAtmosphereRenderFlag::E_EnableAll;
	// Should be same as UpdateAtmosphericFogTransform
	GroundOffset += InComponent->GetComponentLocation().Z;
	FMatrix WorldToLight = InComponent->ComponentToWorld.ToMatrixNoScale().Inverse();
	DefaultSunDirection = FVector(WorldToLight.M[0][0],WorldToLight.M[1][0],WorldToLight.M[2][0]);
}

FAtmosphericFogSceneInfo::~FAtmosphericFogSceneInfo()
{
#if WITH_EDITORONLY_DATA
	if (AtmosphereTextures)
	{
		delete AtmosphereTextures;
		AtmosphereTextures = NULL;
	}
#endif
}

/** A mip bias applied globally to all textures. */
static int32 GAtmosphere = 1;

/**
 * Sets the global texture mip bias and forces all textures to update and stream as needed.
 */
static void SetAtmosphere(const TArray<FString>& Args)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (Args.Num() == 1 && Args[0].Len() > 0)
	{
		int32 NewAtmosphere = FPlatformString::Atoi(*Args[0]) > 0 ? 1 : 0;

		if (GAtmosphere != NewAtmosphere )
		{
			GAtmosphere = NewAtmosphere;

			IConsoleVariable* ICVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AtmosphereRender"));
			if(ICVar)
			{
				ICVar->Set(GAtmosphere);
			}

			for(TObjectIterator<UAtmosphericFogComponent> It; It; ++It)
			{
				UAtmosphericFogComponent* Comp = *It;
				if (!Comp->GetOuter()->HasAnyFlags(RF_ClassDefaultObject))
				{
					if (GAtmosphere > 0 && Comp->IsRegistered())
					{
						Comp->InitResource(); // Initialize resource
					}
					else
					{
						Comp->ReleaseResource();
					}
				}
			}
		}

		UE_LOG(LogConsoleResponse,Display,TEXT("r.Atmosphere %s"), GAtmosphere ? TEXT("Enabled") : TEXT("Disabled"));
	}
#endif
}

/** Console command for setting the global texture mip bias. */
static FAutoConsoleCommand GAtmosphereConsoleCmd(
	TEXT("r.Atmosphere"),
	TEXT("Enable/Disable Atmosphere, Load/Unload related data."),
	FConsoleCommandWithArgsDelegate::CreateStatic(&SetAtmosphere)
	);

bool ShouldRenderAtmosphere(const FSceneViewFamily& Family)
{
	const FEngineShowFlags EngineShowFlags = Family.EngineShowFlags;

	return GAtmosphere
		&& GSupportsVolumeTextureRendering
		&& EngineShowFlags.Atmosphere
		&& !EngineShowFlags.ShaderComplexity
		&& !EngineShowFlags.StationaryLightOverlap 
		&& !EngineShowFlags.LightMapDensity;
}
