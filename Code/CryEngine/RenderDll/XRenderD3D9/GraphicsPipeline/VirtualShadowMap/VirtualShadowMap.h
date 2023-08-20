//TanGram
#pragma once
#include "../Common/GraphicsPipelineStage.h"
#include "../Common/FullscreenPass.h"
#include "../Common/ComputeRenderPass.h"
#include "../Common/GraphicsPipeline.h"

#define PHYSICAL_VSM_TEX_SIZE 2048
#define VIRTUAL_VSM_TEX_SIZE 8192

#define VSM_TILE_SIZE 256
#define VSM_TILE_NUM_WIDTH (PHYSICAL_VSM_TEX_SIZE / VSM_TILE_SIZE)//8 


#define TILE_MASK_CS_GROUP_SIZE 16


struct CVSMParameters
{
	CTexture* m_texDepth;
};

class CVirtualShadowMapStage : public CGraphicsPipelineStage
{
public:
	static const EGraphicsPipelineStage StageID = eStage_VSM;

	CVirtualShadowMapStage(CGraphicsPipeline& graphicsPipeline)
		: CGraphicsPipelineStage(graphicsPipeline)
		, m_passVSMTileMask(&graphicsPipeline, CComputeRenderPass::eFlags_ReflectConstantBuffersFromShader)
		, m_passBufferVisualize(&graphicsPipeline)
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

	void MaskVSMTile();
	void VisualizeBuffer();

private:
	CComputeRenderPass	m_passVSMTileMask;
	CGpuBuffer m_vsmTileMaskBuffer;
	_smart_ptr<CTexture>	m_pShadowDepthTarget;

	CVSMParameters m_vsmParameters;

	CFullscreenPass m_passBufferVisualize;
	CTexture* m_pVisualizeTarget;
};