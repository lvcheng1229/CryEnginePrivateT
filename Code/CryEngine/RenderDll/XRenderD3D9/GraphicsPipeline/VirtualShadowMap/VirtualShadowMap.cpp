#include "VirtualShadowMap.h"



void CVirtualShadowMapStage::Init()
{
	if (!m_vsmTileMaskBuffer.IsAvailable())
	{
		m_vsmTileMaskBuffer.Create(VSM_TILE_NUM_WIDTH * VSM_TILE_NUM_WIDTH, sizeof(uint32), DXGI_FORMAT_R32_UINT, CDeviceObjectFactory::BIND_SHADER_RESOURCE | CDeviceObjectFactory::BIND_UNORDERED_ACCESS, NULL);
		m_vsmTileMaskBuffer.SetDebugName("m_vsmTileMaskBuffer");
	}
}

void CVirtualShadowMapStage::Update()
{
	PrePareShadowMap();
}

//PrepareOutputsForPass
void CVirtualShadowMapStage::PrePareShadowMap()
{
	_smart_ptr<CTexture> pDepthTarget = CTexture::GetOrCreateTextureObjectPtr("VSMShadowMap", PHYSICAL_VSM_TEX_SIZE, PHYSICAL_VSM_TEX_SIZE, 1, eTT_2D, FT_USAGE_TEMPORARY | FT_NOMIPS | FT_STATE_CLAMP, eTF_R32F);

	if (!CTexture::IsTextureExist(pDepthTarget))
	{
		const ColorF cClear(0, 0, 0);
		pDepthTarget->CreateDepthStencil(eTF_R32F, cClear);
	}
	
	m_pShadowDepthTarget = pDepthTarget;
}

void CVirtualShadowMapStage::Execute(CVSMParameters* vsmParameters)
{
	m_vsmParameters = *vsmParameters;

	MaskVSMTile();
	VisualizeBuffer();

	return;
}

void CVirtualShadowMapStage::MaskVSMTile()
{
	m_passVSMTileMask.SetTechnique(CShaderMan::s_ShaderVSM, CCryNameTSCRC("VSMTileMask"), 0);
	m_passVSMTileMask.SetOutputUAV(0, &m_vsmTileMaskBuffer);
	m_passVSMTileMask.SetTexture(0, m_vsmParameters.m_texDepth);

	Vec4i DispatchSize = Vec4i(divideRoundUp(Vec2i(m_vsmParameters.m_texDepth->GetWidth(), m_vsmParameters.m_texDepth->GetHeight()), Vec2i(TILE_MASK_CS_GROUP_SIZE, TILE_MASK_CS_GROUP_SIZE)), 0, 0);

	m_passVSMTileMask.BeginConstantUpdate();
	m_passVSMTileMask.SetConstant(CCryNameR("View_BufferSizeAndInvSize"), Vec4(512, 512, 1.0 / 512.0, 1.0 / 512.0));

	m_passVSMTileMask.SetDispatchSize(DispatchSize.x, DispatchSize.y, 1);
	m_passVSMTileMask.PrepareResourcesForUse(GetDeviceObjectFactory().GetCoreCommandList());

	const bool bAsynchronousCompute = false;
	SScopedComputeCommandList computeCommandList(bAsynchronousCompute);
	m_passVSMTileMask.Execute(computeCommandList);

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
		m_passBufferVisualize.SetBuffer(0, &m_vsmTileMaskBuffer);
	}

	m_passBufferVisualize.BeginConstantUpdate();
	m_passBufferVisualize.SetConstant(CCryNameR("visualizeBufferSize"), Vec4(VSM_TILE_NUM_WIDTH, VSM_TILE_NUM_WIDTH, 1.0, 1.0), eHWSC_Pixel);
	m_passBufferVisualize.Execute();
}
