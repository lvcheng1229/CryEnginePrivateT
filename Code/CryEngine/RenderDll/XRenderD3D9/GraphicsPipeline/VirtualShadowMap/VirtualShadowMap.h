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


struct VSMpFrustum : public CMultiThreadRefCount
{

};

struct CTileFlagGenParameter
{
	Matrix44 invViewProj;
	Matrix44 lightViewProj;
	Vec4 deviceZTexSize;
	Vec4i vsmTileNum;
};

class CVirtualShadowMapManager
{
public:
	CVirtualShadowMapManager()
		:m_frustumValid(false)
	{

	}

	bool m_frustumValid;
	CRenderView* m_pRenderView;
	CTexture* m_texDeviceZ;

	Matrix44A m_lightViewProjMatrix;
};

class CTileFlagGenStage
{
public:
	CTileFlagGenStage(CVirtualShadowMapManager* vsmManager, CComputeRenderPass* compRenderPass)
		:m_vsmManager(vsmManager),
		m_compPass(compRenderPass){};

	void Init();
	void Update();
	void Execute();

	CGpuBuffer* GetVsmTileFlagBuffer()
	{
		return &m_vsmTileFlagBuffer;
	}
private:
	Vec2i m_texDeviceZWH;

	CVirtualShadowMapManager* m_vsmManager;

	CComputeRenderPass* m_compPass;

	CGpuBuffer m_vsmTileFlagBuffer;

	CConstantBufferPtr  m_tileFlagGenConstantBuffer;

	CTileFlagGenParameter m_tileFlagGenParameters;
};


class CTileTableGenStage
{

};


class CVirtualShadowMapStage : public CGraphicsPipelineStage
{
public:
	static const EGraphicsPipelineStage StageID = eStage_VSM;

	CVirtualShadowMapStage(CGraphicsPipeline& graphicsPipeline)
		: CGraphicsPipelineStage(graphicsPipeline)
		, m_passVSMTileFlagGen(&graphicsPipeline)
		, m_passBufferVisualize(&graphicsPipeline)
		, tileFlagGenStage(&m_vsmManager, &m_passVSMTileFlagGen)
	{
		
	}

	bool IsStageActive(EShaderRenderingFlags flags) const final
	{
		return false;
		//return CRenderer::CV_r_VirtualShadowMap;
	}

	void  Init()   final;
	void  Update() final;

	void PrePareShadowMap();

	void Execute();

	void PrepareShadowPassForFrustum();

	void VisualizeBuffer();

private:
	CVirtualShadowMapManager m_vsmManager;

	

	CComputeRenderPass	m_passVSMTileFlagGen;
	CTileFlagGenStage tileFlagGenStage;
	

	_smart_ptr<CTexture>	m_pShadowDepthTarget;
	
	CFullscreenPass m_passBufferVisualize;
	CTexture* m_pVisualizeTarget;
};