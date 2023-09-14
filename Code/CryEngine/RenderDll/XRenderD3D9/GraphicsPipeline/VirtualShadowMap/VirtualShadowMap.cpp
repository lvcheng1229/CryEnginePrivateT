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


void CShadowProjectStage::InitIndirectLayout()
{
	m_deviceResourceIndirectLayoutDesc.m_indirectLayoutTokens.push_back(SDeviceResourceIndirectLayoutToken{ SDeviceResourceIndirectLayoutToken::ETokenType::TT_ConstantBuffer });
	m_deviceResourceIndirectLayoutDesc.m_indirectLayoutTokens.push_back(SDeviceResourceIndirectLayoutToken{ SDeviceResourceIndirectLayoutToken::ETokenType::TT_VertexBuffer });
	m_deviceResourceIndirectLayoutDesc.m_indirectLayoutTokens.push_back(SDeviceResourceIndirectLayoutToken{ SDeviceResourceIndirectLayoutToken::ETokenType::TT_IndexBuffer });
	m_deviceResourceIndirectLayoutDesc.m_indirectLayoutTokens.push_back(SDeviceResourceIndirectLayoutToken{ SDeviceResourceIndirectLayoutToken::ETokenType::TT_DrawIndexd });
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


