#include "VirtualShadowMap.h"



void CTileFlagGenState::Init()
{
	if (!m_vsmTileFlagBuffer.IsAvailable())
	{
		m_vsmTileFlagBuffer.Create(VSM_TILE_NUM_WIDTH * VSM_TILE_NUM_WIDTH, sizeof(uint32), DXGI_FORMAT_R32_UINT, CDeviceObjectFactory::BIND_SHADER_RESOURCE | CDeviceObjectFactory::BIND_UNORDERED_ACCESS, NULL);
		m_vsmTileFlagBuffer.SetDebugName("m_vsmTileFlagBuffer");
	}

	m_tileFlagGenConstantBuffer = gcpRendD3D->m_DevBufMan.CreateConstantBuffer(sizeof(CTileFlagGenParameter));
}

void CTileFlagGenState::Update()
{
	//const Vec3 camPos = pMainView->GetCamera(CCamera::eEye_Left).GetPosition();
	//AABB aabb = frustum.aabbCasters;
	//aabb.Move(camPos);
	//
	//Matrix44A view, proj;
	//CShadowUtils::GetShadowMatrixForObject(proj, view, frustumInfo, frustum.vLightSrcRelPos, aabb);
	//
	//viewProj = view * proj;

	//targetPass.m_ViewProjMatrix = viewProj;
	//targetFrustum.mLightViewMatrix = sourcePass.m_ViewProjMatrix;
	//lightViewProj = pFr->mLightViewMatrix;
	m_tileFlagGenParameters.lightViewProj;

	const SRenderViewInfo& viewInfo = m_vsmManager->m_pRenderView->GetViewInfo(CCamera::eEye_Left);
	m_tileFlagGenParameters.invViewProj = viewInfo.invCameraProjMatrix.GetTransposed();

	m_texDeviceZWH = Vec2i(m_vsmManager->m_texDeviceZ->GetWidth(), m_vsmManager->m_texDeviceZ->GetHeight());
	m_tileFlagGenParameters.deviceZTexSize = Vec4(m_texDeviceZWH, 1.0f / m_texDeviceZWH.x, 1.0f / m_texDeviceZWH.y);

	m_tileFlagGenParameters.vsmTileNum = Vec4i(TILE_MASK_CS_GROUP_SIZE, TILE_MASK_CS_GROUP_SIZE, 0, 0);
	m_tileFlagGenConstantBuffer->UpdateBuffer(&m_tileFlagGenParameters, sizeof(CTileFlagGenParameter), 0, 1);
}

void CTileFlagGenState::Execute()
{
	Vec4i DispatchSize = Vec4i(divideRoundUp(m_texDeviceZWH, Vec2i(TILE_MASK_CS_GROUP_SIZE, TILE_MASK_CS_GROUP_SIZE)), 0, 0);
	
	m_compPass->SetTechnique(CShaderMan::s_ShaderVSM, CCryNameTSCRC("VSMTileFlagGen"), 0);
	m_compPass->SetOutputUAV(0, &m_vsmTileFlagBuffer);
	m_compPass->SetTexture(0, m_vsmManager->m_texDeviceZ);
	m_compPass->SetConstantBuffer(0, m_tileFlagGenConstantBuffer);
	m_compPass->SetDispatchSize(DispatchSize.x, DispatchSize.y, 1);
	m_compPass->PrepareResourcesForUse(GetDeviceObjectFactory().GetCoreCommandList());

	const bool bAsynchronousCompute = false;
	SScopedComputeCommandList computeCommandList(bAsynchronousCompute);
	m_compPass->Execute(computeCommandList);
}







void CVirtualShadowMapStage::Init()
{
	tileFlagGenStage.Init();
}

void CVirtualShadowMapStage::Update()
{
	m_vsmManager.m_pRenderView = RenderView();
	m_vsmManager.m_texDeviceZ = RenderView()->GetDepthTarget();;
	tileFlagGenStage.Update();
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

void CVirtualShadowMapStage::Execute(CVSMParameters* vsmParameters)
{
	tileFlagGenStage.Execute();

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
		m_passBufferVisualize.SetBuffer(0, tileFlagGenStage.GetVsmTileFlagBuffer());
	}

	m_passBufferVisualize.BeginConstantUpdate();
	m_passBufferVisualize.SetConstant(CCryNameR("visualizeBufferSize"), Vec4(VSM_TILE_NUM_WIDTH, VSM_TILE_NUM_WIDTH, 1.0, 1.0), eHWSC_Pixel);
	m_passBufferVisualize.Execute();
}


