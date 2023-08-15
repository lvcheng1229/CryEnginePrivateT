//TanGram
#pragma once
#include "../Common/GraphicsPipelineStage.h"
#include "../Common/FullscreenPass.h"
#include "../Common/ComputeRenderPass.h"
#include "../Common/GraphicsPipeline.h"

class CVirtualShadowMapStage : public CGraphicsPipelineStage
{
public:
	static const EGraphicsPipelineStage StageID = eStage_VSM;

	CVirtualShadowMapStage(CGraphicsPipeline& graphicsPipeline)
		: CGraphicsPipelineStage(graphicsPipeline)
	{

	}

	bool IsStageActive(EShaderRenderingFlags flags) const final
	{
		return CRenderer::CV_r_VirtualShadowMap;
	}

	void Execute();
};