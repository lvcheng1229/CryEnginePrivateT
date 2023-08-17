//TanGram
#pragma once
#include "../Common/GraphicsPipelineStage.h"
#include "../Common/FullscreenPass.h"
#include "../Common/ComputeRenderPass.h"
#include "../Common/GraphicsPipeline.h"

static const int32 VSMTexSizeX = 2048;
static const int32 VSMTexSizeY = 2048;

#define TILE_MASK_CS_GROUP_SIZE 16
#define VIRTUAL_TILE_NUM_WIDTH 8

struct CVSMParameters
{
	CTexture* DepthTexIn;
};

class CVirtualShadowMapStage : public CGraphicsPipelineStage
{
public:
	static const EGraphicsPipelineStage StageID = eStage_VSM;

	CVirtualShadowMapStage(CGraphicsPipeline& graphicsPipeline)
		: CGraphicsPipelineStage(graphicsPipeline)
		, m_passVSMTileMask(&graphicsPipeline, CComputeRenderPass::eFlags_ReflectConstantBuffersFromShader)
	{

	}

	bool IsStageActive(EShaderRenderingFlags flags) const final
	{
		return CRenderer::CV_r_VirtualShadowMap;
	}

	void  Init()   final;
	void  Update() final;

	void PrePareShadowMap();

	void Execute(CVSMParameters* vsmParameters);
private:
	CComputeRenderPass	m_passVSMTileMask;

	CGpuBuffer m_vsmTileMaskBuffer;

	_smart_ptr<CTexture>	m_pShadowDepthTarget;
};