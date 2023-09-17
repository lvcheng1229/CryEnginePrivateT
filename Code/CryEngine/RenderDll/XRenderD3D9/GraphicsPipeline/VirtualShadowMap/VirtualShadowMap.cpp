#include "VirtualShadowMap.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// vsm tile flag generation stage ////////////////////////////////////////////////////////////////////////

CTileFlagGenStage::~CTileFlagGenStage()
{
	m_vsmTileFlagBuffer.Release();
	m_tileFlagGenConstantBuffer.reset();
}

void CTileFlagGenStage::Init()
{
	if (!m_vsmTileFlagBuffer.IsAvailable())
	{
		m_vsmTileFlagBuffer.Create(VSM_VIRTUAL_TILE_SIZE_WH * VSM_VIRTUAL_TILE_SIZE_WH, sizeof(uint32), DXGI_FORMAT_R32_UINT, CDeviceObjectFactory::BIND_SHADER_RESOURCE | CDeviceObjectFactory::BIND_UNORDERED_ACCESS, NULL);
		m_vsmTileFlagBuffer.SetDebugName("m_vsmTileFlagBuffer");

		m_vsmGlobalInfo->m_vsmTileFlagBuffer = &m_vsmTileFlagBuffer;
	}

	m_tileFlagGenConstantBuffer = gcpRendD3D->m_DevBufMan.CreateConstantBuffer(sizeof(CTileFlagGenParameter));
}

void CTileFlagGenStage::Update()
{
	m_tileFlagGenParameters.lightViewProj = m_vsmGlobalInfo->m_lightViewProjMatrix.GetTransposed();

	const SRenderViewInfo& viewInfo = m_vsmGlobalInfo->m_pRenderView->GetViewInfo(CCamera::eEye_Left);
	m_tileFlagGenParameters.invViewProj = viewInfo.invCameraProjMatrix.GetTransposed();

	m_texDeviceZWH = Vec2i(m_vsmGlobalInfo->m_texDeviceZ->GetWidth(), m_vsmGlobalInfo->m_texDeviceZ->GetHeight());
	m_tileFlagGenParameters.deviceZTexSize = Vec4(m_texDeviceZWH, 1.0f / m_texDeviceZWH.x, 1.0f / m_texDeviceZWH.y);

	m_tileFlagGenParameters.vsmVirtualTileSizeWH = Vec4i(VSM_VIRTUAL_TILE_SIZE_WH, VSM_VIRTUAL_TILE_SIZE_WH, 0, 0);
	m_tileFlagGenConstantBuffer->UpdateBuffer(&m_tileFlagGenParameters, sizeof(CTileFlagGenParameter), 0, 1);
}

void CTileFlagGenStage::Execute()
{
	PROFILE_LABEL_SCOPE("VSM_TILE_FLAG_GEN");

	CClearSurfacePass::Execute(&m_vsmTileFlagBuffer, ColorI(0, 0, 0, 0));

	Vec4i DispatchSize = Vec4i(divideRoundUp(m_texDeviceZWH, Vec2i(TILE_MASK_CS_GROUP_SIZE, TILE_MASK_CS_GROUP_SIZE)), 0, 0);

	m_compPass->SetTechnique(CShaderMan::s_ShaderVSM, CCryNameTSCRC("VSMTileFlagGen"), 0);
	m_compPass->SetOutputUAV(0, &m_vsmTileFlagBuffer);
	m_compPass->SetTexture(0, m_vsmGlobalInfo->m_texDeviceZ);
	m_compPass->SetConstantBuffer(0, m_tileFlagGenConstantBuffer);
	m_compPass->SetDispatchSize(DispatchSize.x, DispatchSize.y, 1);
	m_compPass->PrepareResourcesForUse(GetDeviceObjectFactory().GetCoreCommandList());

	const bool bAsynchronousCompute = false;
	SScopedComputeCommandList computeCommandList(bAsynchronousCompute);
	m_compPass->Execute(computeCommandList);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// vsm tile table generation stage ////////////////////////////////////////////////////////////////////////

void CTileTableGenStage::Init()
{
	if (!m_vsmTileTableBuffer.IsAvailable())
	{
		m_vsmTileTableBuffer.Create(VSM_PHYSICAL_TILE_SIZE_WH * VSM_PHYSICAL_TILE_SIZE_WH, sizeof(uint32), DXGI_FORMAT_UNKNOWN, CDeviceObjectFactory::USAGE_STRUCTURED | CDeviceObjectFactory::BIND_UNORDERED_ACCESS, NULL);
		m_vsmTileTableBuffer.SetDebugName("m_vsmTileTableBuffer");
	}

	if (!m_vsmValidTileCountBuffer.IsAvailable())
	{
		m_vsmValidTileCountBuffer.Create(1, sizeof(uint32), DXGI_FORMAT_UNKNOWN, CDeviceObjectFactory::USAGE_STRUCTURED | CDeviceObjectFactory::BIND_UNORDERED_ACCESS, NULL);
		m_vsmValidTileCountBuffer.SetDebugName("m_vsmValidTileCountBuffer");
	}

	m_tileTableGenConstantBuffer = gcpRendD3D->m_DevBufMan.CreateConstantBuffer(sizeof(CTileTableGenParameter));
}

void CTileTableGenStage::Update()
{
	m_tileTableGenParameters.vsmVirtualTileSizeWH = Vec4i(VSM_VIRTUAL_TILE_SIZE_WH, VSM_VIRTUAL_TILE_SIZE_WH, 0, 0);
	m_tileTableGenParameters.vsmPhyTileSizeWH = Vec4i(VSM_PHYSICAL_TILE_SIZE_WH, VSM_PHYSICAL_TILE_SIZE_WH, 0, 0);
	m_tileTableGenConstantBuffer->UpdateBuffer(&m_tileTableGenParameters, sizeof(CTileTableGenParameter), 0, 1);
}

void CTileTableGenStage::Execute()
{
	PROFILE_LABEL_SCOPE("VSM_TILE_TABLE_GEN");

	Vec4i DispatchSize = Vec4i(divideRoundUp(Vec2i(VSM_VIRTUAL_TILE_SIZE_WH, VSM_VIRTUAL_TILE_SIZE_WH), Vec2i(TILE_TABLE_GEN_CS_GROUP_SIZE, TILE_TABLE_GEN_CS_GROUP_SIZE)), 0, 0);

	m_compPass->SetTechnique(CShaderMan::s_ShaderVSM, CCryNameTSCRC("VSMTileTableGen"), 0);
	m_compPass->SetOutputUAV(0, m_vsmGlobalInfo->m_vsmTileFlagBuffer);
	m_compPass->SetOutputUAV(1, &m_vsmTileTableBuffer);
	m_compPass->SetOutputUAV(2, &m_vsmValidTileCountBuffer);
	m_compPass->SetConstantBuffer(0, m_tileTableGenConstantBuffer);
	m_compPass->SetDispatchSize(DispatchSize.x, DispatchSize.y, 1);
	m_compPass->PrepareResourcesForUse(GetDeviceObjectFactory().GetCoreCommandList());
	
	const bool bAsynchronousCompute = false;
	SScopedComputeCommandList computeCommandList(bAsynchronousCompute);
	m_compPass->Execute(computeCommandList);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// vsm projection stage ////////////////////////////////////////////////////////////////////////

void CShadowProjectStage::InitIndirectLayout()
{
	m_deviceResourceIndirectLayoutDesc.m_indirectLayoutTokens.push_back(SDeviceResourceIndirectLayoutToken{ SDeviceResourceIndirectLayoutToken::ETokenType::TT_ConstantBuffer });
	m_deviceResourceIndirectLayoutDesc.m_indirectLayoutTokens.push_back(SDeviceResourceIndirectLayoutToken{ SDeviceResourceIndirectLayoutToken::ETokenType::TT_VertexBuffer });
	m_deviceResourceIndirectLayoutDesc.m_indirectLayoutTokens.push_back(SDeviceResourceIndirectLayoutToken{ SDeviceResourceIndirectLayoutToken::ETokenType::TT_IndexBuffer });
	m_deviceResourceIndirectLayoutDesc.m_indirectLayoutTokens.push_back(SDeviceResourceIndirectLayoutToken{ SDeviceResourceIndirectLayoutToken::ETokenType::TT_DrawIndexd });
}

void CShadowProjectStage::Execute()
{
	//CRenderView* pRenderView = m_vsmGlobalInfo->m_pRenderView;
	//CRenerItemGPUDrawer& renderItemGPUDrawer = pRenderView->GetGPUDrawer();

	m_vsmShadowProjectPass.BeginExecution(m_graphicsPipeline);
	//m_vsmShadowProjectPass.SetupDrawContext(StageID, curPass.m_eShadowPassID, TTYPE_SHADOWGEN, 0);
	//m_vsmShadowProjectPass.DrawRenderItems(pShadowsView, (ERenderListID)curPass.m_nShadowFrustumSide);
	//m_vsmShadowProjectPass.EndExecution();

	//CRenderItemDrawer& rendItemDrawer = pRenderView->GetDrawer();
	//rendItemDrawer.InitDrawSubmission();
	//rendItemDrawer.JobifyDrawSubmission();
	//rendItemDrawer.WaitForDrawSubmission(); disable multi thread
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// vsm tile flag generation stage ////////////////////////////////////////////////////////////////////////

void CVirtualShadowMapStage::Init()
{
	m_tileFlagGenStage.Init();
	m_tileTableGenStage.Init();
}

void CVirtualShadowMapStage::Update()
{
	Matrix44A viewMatrix, projMatrix;
	CRenderView* pRenderView = RenderView();
	m_vsmGlobalInfo.m_pRenderView = pRenderView;
	m_vsmGlobalInfo.m_texDeviceZ = pRenderView->GetDepthTarget();

	SShadowFrustumToRender* pVSMFrustum = nullptr;
	for (auto& fr : pRenderView->m_shadows.m_renderFrustums)
	{
		if (fr.pFrustum->m_eFrustumType == ShadowMapFrustum::e_VSM)
		{
			pVSMFrustum = &fr;
			break;
		}
	}

	if (!pVSMFrustum)
	{
		m_vsmGlobalInfo.m_frustumValid = false;
		return;
	}

	//m_light.m_Origin = passInfo.GetCamera().GetPosition() + GetSunDir()
	//vLightSrcRelPos = m_light.m_Origin - passInfo.GetCamera().GetPosition() = GetSunDir()

	f32 aabbRadiusCam = 1000.0f;
	f32 w = aabbRadiusCam;
	f32 h = w;
	f32 zn = 0.1f;
	f32 zf = aabbRadiusCam * 2.0f;

	mathMatrixOrtho(&projMatrix, w, h, zn, zf);
	
	const Vec3 zAxis(0.f, 0.f, 1.f);
	const Vec3 yAxis(0.f, 1.f, 0.f);
	Vec3 up;
	Vec3 camPos = pRenderView->GetCamera(CCamera::eEye_Left).GetPosition();
	Vec3 at = camPos;
	Vec3 vLightDir = -pVSMFrustum->pFrustum->vLightSrcRelPos;
	vLightDir.Normalize();
	
	Vec3 eye = at - pVSMFrustum->pFrustum->vLightSrcRelPos.len() * vLightDir;
	
	if (fabsf(vLightDir.Dot(zAxis)) > 0.9995f)
		up = yAxis;
	else
		up = zAxis;
	
	mathMatrixLookAt(&viewMatrix, eye, at, up);

	m_vsmGlobalInfo.m_lightViewProjMatrix = viewMatrix * projMatrix;
	m_vsmGlobalInfo.m_frustumValid = true;

	for (auto& fr : pRenderView->m_shadows.m_renderFrustums)
	{
		auto pShadowView = reinterpret_cast<CRenderView*>(fr.pShadowsView.get());
		pShadowView->SwitchUsageMode(IRenderView::eUsageModeReading);
	}

	m_tileFlagGenStage.Update();
	m_tileTableGenStage.Update();
}



//PrepareOutputsForPass
void CVirtualShadowMapStage::PrePareShadowMap()
{
	//_smart_ptr<CTexture> pDepthTarget = CTexture::GetOrCreateTextureObjectPtr("VSMShadowMap", PHYSICAL_VSM_TEX_SIZE, PHYSICAL_VSM_TEX_SIZE, 1, eTT_2D, FT_USAGE_TEMPORARY | FT_NOMIPS | FT_STATE_CLAMP, eTF_R32F);
	//
	//if (!CTexture::IsTextureExist(pDepthTarget))
	//{
	//	const ColorF cClear(0, 0, 0);
	//	pDepthTarget->CreateDepthStencil(eTF_R32F, cClear);
	//}
	//
	//m_pShadowDepthTarget = pDepthTarget;
}

void CVirtualShadowMapStage::Execute()
{
	if (m_vsmGlobalInfo.m_frustumValid)
	{
		m_tileFlagGenStage.Execute();
		m_tileTableGenStage.Execute();
	}

	VisualizeBuffer();

	return;
}

bool CVirtualShadowMapStage::CreatePipelineStateInner(const SGraphicsPipelineStateDescription& description, CDeviceGraphicsPSOPtr& outPSO)
{
//	outPSO = NULL;
//
//	CShader* pShader = static_cast<CShader*>(description.shaderItem.m_pShader);
//	SShaderTechnique* pTechnique = pShader->GetTechnique(description.shaderItem.m_nTechnique, description.technique, true);
//	if (!pTechnique)
//		return true;
//
//	CShaderResources* pRes = static_cast<CShaderResources*>(description.shaderItem.m_pShaderResources);
//	if (pRes->m_ResFlags & MTL_FLAG_NOSHADOW)
//		return true;
//
//	SShaderPass* pShaderPass = &pTechnique->m_Passes[0];
//	uint64 objectFlags = description.objectFlags;
//
//	CDeviceGraphicsPSODesc psoDesc(m_pResourceLayout, description);
//	psoDesc.m_bDynamicDepthBias = true;
//
//	// Handle quality flags
//	CGraphicsPipeline::ApplyShaderQuality(psoDesc, gcpRendD3D->GetShaderProfile(pShader->m_eShaderType));
//
//
//	// Set resource states
//	bool bTwoSided = false;
//	{
//		if (pRes->m_ResFlags & MTL_FLAG_2SIDED)
//			bTwoSided = true;
//
//		if (pRes->IsAlphaTested())
//			psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_ALPHATEST];
//
//		if (passID == ePass_DirectionalLightRSM || passID == ePass_LocalLightRSM)
//		{
//			if (pRes->m_Textures[EFTT_DIFFUSE] && pRes->m_Textures[EFTT_DIFFUSE]->m_Ext.m_pTexModifier)
//				psoDesc.m_ShaderFlags_MD |= pRes->m_Textures[EFTT_DIFFUSE]->m_Ext.m_nUpdateFlags;
//		}
//
//		// Merge EDeformType into EVertexModifier to save space/parameters
//		if (pRes->m_pDeformInfo)
//			psoDesc.m_ShaderFlags_MDV |= EVertexModifier(pRes->m_pDeformInfo->m_eType);
//	}
//
//	if (m_shadowsLocalLightsLinearizeDepth == 1)
//	{
//		psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_SHADOW_DEPTH_OUTPUT_LINEAR];
//	}
//
//	//tessellation
//	psoDesc.m_bAllowTesselation = false;
//	psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_NO_TESSELLATION];
//
//#ifdef TESSELLATION_RENDERER
//	const bool bHasTesselationShaders = pShaderPass && pShaderPass->m_HShader && pShaderPass->m_DShader;
//	if (bHasTesselationShaders && (!(objectFlags & FOB_NEAREST) && (objectFlags & FOB_ALLOW_TESSELLATION)))
//	{
//		psoDesc.m_ShaderFlags_RT &= ~g_HWSR_MaskBit[HWSR_NO_TESSELLATION];
//		psoDesc.m_bAllowTesselation = true;
//	}
//#endif
//
//	psoDesc.m_CullMode = bTwoSided ? eCULL_None : ((pShaderPass && pShaderPass->m_eCull != -1) ? (ECull)pShaderPass->m_eCull : eCULL_Back);
//	if (pShader->m_eSHDType == eSHDT_Terrain)
//	{
//		//Flipped matrix for point light sources
//		if (passID == ePass_DirectionalLight || passID == ePass_DirectionalLightCached)
//			psoDesc.m_CullMode = eCULL_None;
//		else
//			psoDesc.m_CullMode = eCULL_Front; //front faces culling by default for terrain
//	}
//
//	if (passID == ePass_DirectionalLight || passID == ePass_DirectionalLightCached || passID == ePass_DirectionalLightRSM)
//	{
//		psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_HW_PCF_COMPARE];
//	}
//	else if (passID == ePass_LocalLight || passID == ePass_LocalLightRSM)
//	{
//		psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_HW_PCF_COMPARE];
//		psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_CUBEMAP0];
//
//		// RBPF_MIRRORCULL
//		if (psoDesc.m_CullMode != eCULL_None)
//		{
//			psoDesc.m_CullMode = (psoDesc.m_CullMode == eCULL_Front) ? eCULL_Back : eCULL_Front;
//		}
//	}
//
//	if (passID == ePass_DirectionalLightRSM || passID == ePass_LocalLightRSM)
//	{
//		psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_SAMPLE4];
//
//		if (!bTwoSided && psoDesc.m_CullMode == eCULL_Front)
//			psoDesc.m_CullMode = eCULL_Back;
//
//		if (objectFlags & FOB_DECAL_TEXGEN_2D)
//			psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_DECAL_TEXGEN_2D];
//
//		if ((objectFlags & FOB_BLEND_WITH_TERRAIN_COLOR)) // && rRP.m_pCurObject->m_nTextureID > 0
//			psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_BLEND_WITH_TERRAIN_COLOR];
//	}
//
//	psoDesc.m_ShaderFlags_MDV |= pShader->m_nMDV;
//	if (objectFlags & FOB_OWNER_GEOMETRY)
//		psoDesc.m_ShaderFlags_MDV &= ~MDV_DEPTH_OFFSET;
//	if (objectFlags & FOB_BENDED)
//		psoDesc.m_ShaderFlags_MDV |= MDV_BENDING;
//
//	if (!(objectFlags & FOB_TRANS_MASK))  //&& gRenDev->m_RP.m_RIs[0].Num() <= 1
//		psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_OBJ_IDENTITY];
//
//	if (objectFlags & FOB_NEAREST)
//		psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_NEAREST];
//	if (objectFlags & FOB_DISSOLVE)
//		psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_DISSOLVE];
//	if (psoDesc.m_RenderState & GS_ALPHATEST)
//		psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_ALPHATEST];
//
//	if (psoDesc.m_bAllowTesselation && (psoDesc.m_PrimitiveType < ept1ControlPointPatchList || psoDesc.m_PrimitiveType > ept4ControlPointPatchList))
//	{
//		psoDesc.m_PrimitiveType = ept3ControlPointPatchList;
//		psoDesc.m_ObjectStreamMask |= VSM_NORMALS;
//	}
//
//	// rendertarget and depth stencil format
//	psoDesc.m_pRenderPass = m_ShadowMapPasses[passID][0].GetRenderPass();
//
//#if (CRY_RENDERER_DIRECT3D >= 120)
//	// emulate slope scaled bias in shader
//	if (passID == ePass_DirectionalLight || passID == ePass_DirectionalLightCached || passID == ePass_DirectionalLightRSM)
//	{
//		psoDesc.m_ShaderFlags_RT |= g_HWSR_MaskBit[HWSR_SAMPLE0];
//	}
//#endif
//
//	// Create PSO
//	outPSO = GetDeviceObjectFactory().CreateGraphicsPSO(psoDesc);
//	return outPSO != nullptr;
	return false;
}


bool CVirtualShadowMapStage::CreatePipelineStates(DevicePipelineStatesArray* pStateArray, const SGraphicsPipelineStateDescription& stateDesc, CGraphicsPipelineStateLocalCache* pStateCache)
{
	return true;//TEMP!

	DevicePipelineStatesArray& stageStates = pStateArray[StageID];

	if (pStateCache->Find(stateDesc, stageStates))
		return true;

	bool bFullyCompiled = CreatePipelineStateInner(stateDesc, stageStates[0]);
	if (bFullyCompiled)
	{
		pStateCache->Put(stateDesc, stageStates);
	}

	return bFullyCompiled;
}

void CVirtualShadowMapStage::VisualizeBuffer()
{
	PROFILE_LABEL_SCOPE("VSM_VISUALIZE_BUFFER");

	if (m_passBufferVisualize.IsDirty())
	{
		m_passBufferVisualize.SetPrimitiveFlags(CRenderPrimitive::eFlags_ReflectShaderConstants_PS);
		m_passBufferVisualize.SetPrimitiveType(CRenderPrimitive::ePrim_ProceduralTriangle);
		m_passBufferVisualize.SetTechnique(CShaderMan::s_ShaderVSM, CCryNameTSCRC("VSMVisualizeBuffer"), 0);
		m_passBufferVisualize.SetRenderTarget(0, m_graphicsPipelineResources.m_pTexVSMVisualize);
		m_passBufferVisualize.SetState(GS_NODEPTHTEST | GS_NOCOLMASK_A);
		m_passBufferVisualize.SetBuffer(0, m_tileFlagGenStage.GetVsmTileFlagBuffer());
	}

	m_passBufferVisualize.BeginConstantUpdate();
	m_passBufferVisualize.SetConstant(CCryNameR("visualizeBufferSize"), Vec4(VSM_VIRTUAL_TILE_SIZE_WH, VSM_VIRTUAL_TILE_SIZE_WH, 1.0, 1.0), eHWSC_Pixel);
	m_passBufferVisualize.Execute();
}


