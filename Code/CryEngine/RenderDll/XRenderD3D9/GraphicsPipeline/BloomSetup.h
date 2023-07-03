//TanGram: TiledBloom
#pragma once
#include "Common/GraphicsPipelineStage.h"
#include "Common/FullscreenPass.h"
#include "Common/ComputeRenderPass.h"
#include "Common\GraphicsPipeline.h"

struct STypedBloomSetupConstants
{
	Vec4	TexSize;
};


class CBloomSetupStage : public CGraphicsPipelineStage
{
public:
	static const EGraphicsPipelineStage StageID = eStage_BloomSetup;

	CBloomSetupStage(CGraphicsPipeline& graphicsPipeline)
		: CGraphicsPipelineStage(graphicsPipeline)
		, m_passBloomSetup(&graphicsPipeline, CComputeRenderPass::eFlags_ReflectConstantBuffersFromShader)
		//, m_passBloomSetup(&graphicsPipeline)
	{

	}

	bool IsStageActive(EShaderRenderingFlags flags) const final
	{
		return CRenderer::CV_r_PostProcess;
	}
	void Init() final;
	void Execute(CTexture* pSrcRT, CTexture* pAutoExposureDestRT, CTexture* pTiledBloomDestRT);

private:
	gpu::CTypedConstantBuffer<STypedBloomSetupConstants>  m_parameters;
	CComputeRenderPass m_passBloomSetup;

};
