//TanGram: TiledBloom
#pragma once
#include "Common/GraphicsPipelineStage.h"
#include "Common/FullscreenPass.h"
#include "Common/ComputeRenderPass.h"
#include "Common\GraphicsPipeline.h"

struct STypedBloomSetupConstants
{
	Vec4 TexSize;
	Vec4i BufferSize;
};


class CBloomSetupStage : public CGraphicsPipelineStage
{
public:
	static const EGraphicsPipelineStage StageID = eStage_BloomSetup;

	CBloomSetupStage(CGraphicsPipeline& graphicsPipeline)
		: CGraphicsPipelineStage(graphicsPipeline)
		, m_passBloomSetup(&graphicsPipeline)
		, m_passBloomTileInfo1Gen(&graphicsPipeline)
		, m_pass1H(&graphicsPipeline)
	{

	}

	~CBloomSetupStage();

	bool IsStageActive(EShaderRenderingFlags flags) const final
	{
		return CRenderer::CV_r_PostProcess;
	}

	void Execute(CTexture* pSrcRT, CTexture* pTiledBloomDestRT, CTexture* pTexBloomOutRT);

	void InitBuffer();
private:

	gpu::CTypedConstantBuffer<STypedBloomSetupConstants>  m_parameters;

	CComputeRenderPass m_passBloomSetup;
	CComputeRenderPass m_passBloomTileInfo1Gen;

	CComputeRenderPass m_pass1H;

	CGpuBuffer m_tileBloomMaskBuffer;

	CGpuBuffer m_tileBloomInfoGen_TileInfoBuffer;
	CGpuBuffer m_tileBloomInfoGen_DispatchThreadCount;

	bool bBufferInit = false;
	int width = 0;
	int height = 0;

	const uint32 groupSizeX = 8;
	const uint32 groupSizeY = 8;
};
