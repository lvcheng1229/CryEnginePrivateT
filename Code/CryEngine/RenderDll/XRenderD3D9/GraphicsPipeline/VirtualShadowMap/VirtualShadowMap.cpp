#include "VirtualShadowMap.h"



void CVirtualShadowMapStage::Init()
{
	if (!m_vsmTileMaskBuffer.IsAvailable())
	{
		m_vsmTileMaskBuffer.Create(VIRTUAL_TILE_NUM_WIDTH * VIRTUAL_TILE_NUM_WIDTH, sizeof(uint32), DXGI_FORMAT_R32_UINT, CDeviceObjectFactory::BIND_SHADER_RESOURCE | CDeviceObjectFactory::BIND_UNORDERED_ACCESS, NULL);
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
	const char* pName = {"VSMShadowMap"};
	_smart_ptr<CTexture> pDepthTarget = CTexture::GetOrCreateTextureObjectPtr(pName, VSMTexSizeX, VSMTexSizeY, 1, eTT_2D, FT_USAGE_TEMPORARY | FT_NOMIPS | FT_STATE_CLAMP, eTF_R32F);

	if (!CTexture::IsTextureExist(pDepthTarget))
	{
		const ColorF cClear(0, 0, 0);
		pDepthTarget->CreateDepthStencil(eTF_R32F, cClear);
	}
	
	m_pShadowDepthTarget = pDepthTarget;
}

void CVirtualShadowMapStage::Execute(CVSMParameters* vsmParameters)
{
	m_passVSMTileMask.SetTechnique(CShaderMan::s_ShaderVSM, CCryNameTSCRC("VSMTileMask"), 0);
	m_passVSMTileMask.SetOutputUAV(0, &m_vsmTileMaskBuffer);
	m_passVSMTileMask.SetTexture(0, vsmParameters->DepthTexIn);

	Vec4i DispatchSize = Vec4i(divideRoundUp(Vec2i(vsmParameters->DepthTexIn->GetWidth(), vsmParameters->DepthTexIn->GetHeight()), Vec2i(TILE_MASK_CS_GROUP_SIZE, TILE_MASK_CS_GROUP_SIZE)), 0, 0);

	m_passVSMTileMask.BeginConstantUpdate();
	m_passVSMTileMask.SetConstant(CCryNameR("View_BufferSizeAndInvSize"), Vec4(512, 512, 1.0 / 512.0, 1.0 / 512.0));

	m_passVSMTileMask.SetDispatchSize(DispatchSize.x, DispatchSize.y, 1);
	m_passVSMTileMask.PrepareResourcesForUse(GetDeviceObjectFactory().GetCoreCommandList());

	const bool bAsynchronousCompute = false;
	SScopedComputeCommandList computeCommandList(bAsynchronousCompute);
	m_passVSMTileMask.Execute(computeCommandList);

	return;
}
