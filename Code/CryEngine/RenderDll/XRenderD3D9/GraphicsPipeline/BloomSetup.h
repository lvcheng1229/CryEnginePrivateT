//TanGram: TiledBloom
#pragma once
#include "Common/GraphicsPipelineStage.h"
#include "Common/FullscreenPass.h"
#include "Common/ComputeRenderPass.h"
#include "Common\GraphicsPipeline.h"

class CBloomSetupStage : public CGraphicsPipelineStage
{
public:
	static const EGraphicsPipelineStage StageID = eStage_BloomSetup;

	CBloomSetupStage(CGraphicsPipeline& graphicsPipeline)
		: CGraphicsPipelineStage(graphicsPipeline)
		, m_passBloomSetup(&graphicsPipeline, CComputeRenderPass::eFlags_ReflectConstantBuffersFromShader)
	{

	}

	bool IsStageActive(EShaderRenderingFlags flags) const final
	{
		return CRenderer::CV_r_PostProcess;
	}

	void Execute(CTexture* pSrcRT, CTexture* pAutoExposureDestRT, CTexture* pTiledBloomDestRT);

private:

	CComputeRenderPass m_passBloomSetup;
};
