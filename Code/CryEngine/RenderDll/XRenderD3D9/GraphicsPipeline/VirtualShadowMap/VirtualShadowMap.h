//TanGram
#pragma once
#include "../Common/GraphicsPipelineStage.h"
#include "../Common/FullscreenPass.h"
#include "../Common/ComputeRenderPass.h"
#include "../Common/GraphicsPipeline.h"

#define PHYSICAL_VSM_TEX_SIZE 2048
#define VIRTUAL_VSM_TEX_SIZE 8192

#define VSM_TILE_SIZE 256

#define VSM_PHYSICAL_TILE_SIZE_WH (PHYSICAL_VSM_TEX_SIZE	/ VSM_TILE_SIZE)//8 
#define VSM_VIRTUAL_TILE_SIZE_WH	(VIRTUAL_VSM_TEX_SIZE	/ VSM_TILE_SIZE)//32

#define TILE_MASK_CS_GROUP_SIZE 16
#define TILE_TABLE_GEN_CS_GROUP_SIZE 16




class CVSMGlobalInfo
{
public:
	CVSMGlobalInfo()
		:m_frustumValid(false)
	{

	}



	bool m_frustumValid;
	CRenderView* m_pRenderView;
	

	//used in tile flag generation pass
	CTexture* m_texDeviceZ;
	Matrix44A m_lightViewProjMatrix;

	//used in tile table generation pass
	CGpuBuffer* m_vsmTileFlagBuffer;
};

class CTileFlagGenStage
{
public:
	CTileFlagGenStage(CVSMGlobalInfo* vsmGlobalInfo, CComputeRenderPass* compRenderPass)
		:m_vsmGlobalInfo(vsmGlobalInfo),
		m_compPass(compRenderPass){};

	~CTileFlagGenStage();

	struct CTileFlagGenParameter
	{
		Matrix44 invViewProj;
		Matrix44 lightViewProj;
		Vec4 deviceZTexSize;
		Vec4i vsmVirtualTileSizeWH;
	};

	void Init();
	void Update();
	void Execute();

	CGpuBuffer* GetVsmTileFlagBuffer()
	{
		return &m_vsmTileFlagBuffer;
	}
private:
	CComputeRenderPass* m_compPass;
	CVSMGlobalInfo* m_vsmGlobalInfo;

	//buffers
	CGpuBuffer m_vsmTileFlagBuffer;
	CConstantBufferPtr  m_tileFlagGenConstantBuffer;

	//parameters
	Vec2i m_texDeviceZWH;
	CTileFlagGenParameter m_tileFlagGenParameters;
};


class CTileTableGenStage
{
public:
	CTileTableGenStage(CVSMGlobalInfo* vsmGlobalInfo, CComputeRenderPass* compRenderPass)
		:m_vsmGlobalInfo(vsmGlobalInfo),
		m_compPass(compRenderPass) {};

	void Init();
	void Update();
	void Execute();

	struct CTileTableGenParameter
	{
		Vec4i vsmVirtualTileSizeWH;
		Vec4i vsmPhyTileSizeWH;
	};
private:
	CComputeRenderPass* m_compPass;
	CVSMGlobalInfo* m_vsmGlobalInfo;

	//buffers
	CGpuBuffer m_vsmTileTableBuffer;
	CGpuBuffer m_vsmValidTileCountBuffer;
	CConstantBufferPtr  m_tileTableGenConstantBuffer;

	//parameters
	CTileTableGenParameter m_tileTableGenParameters;
};

class CShadowProjectStage
{
public:
	void InitIndirectLayout();

	SDeviceResourceIndirectLayoutDesc m_deviceResourceIndirectLayoutDesc;
};

class CVirtualShadowMapStage : public CGraphicsPipelineStage
{
public:
	static const EGraphicsPipelineStage StageID = eStage_VSM;

	CVirtualShadowMapStage(CGraphicsPipeline& graphicsPipeline)
		: CGraphicsPipelineStage(graphicsPipeline)
		, m_passVSMTileFlagGen(&graphicsPipeline)
		, m_passVSMTileTableGen(&graphicsPipeline)
		, m_passBufferVisualize(&graphicsPipeline)
		, m_tileFlagGenStage(&m_vsmGlobalInfo, &m_passVSMTileFlagGen)
		, m_tileTableGenStage(&m_vsmGlobalInfo, &m_passVSMTileTableGen)
	{
		
	}

	bool IsStageActive(EShaderRenderingFlags flags) const final
	{
		return false;
		//return CRenderer::CV_r_VirtualShadowMap;
	}

	void  Init()   final;
	void  Update() final;

	void Execute();
	
	void PrePareShadowMap();
	void VisualizeBuffer();

private:
	CVSMGlobalInfo m_vsmGlobalInfo;

	//tile flag generation stage
	CComputeRenderPass	m_passVSMTileFlagGen;
	CTileFlagGenStage m_tileFlagGenStage;

	//tile table generation stage
	CComputeRenderPass m_passVSMTileTableGen;
	CTileTableGenStage m_tileTableGenStage;

	_smart_ptr<CTexture>	m_pShadowDepthTarget;
	
	CFullscreenPass m_passBufferVisualize;
	CTexture* m_pVisualizeTarget;
};