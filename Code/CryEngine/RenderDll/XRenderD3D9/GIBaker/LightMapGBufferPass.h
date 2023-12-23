#pragma once

#include "../GraphicsPipeline/Common/GraphicsPipelineStage.h"
#include "../GraphicsPipeline/Common/GraphicsPipelineStateSet.h"
#include "../GraphicsPipeline/Common/SceneRenderPass.h"
#include "../GraphicsPipeline/Common/FullscreenPass.h"
#include "../GraphicsPipeline/Common/UtilityPasses.h"

struct SAtlasGeometry
{
	SStreamInfo m_posStream;
	SStreamInfo m_normStream;
	SStreamInfo m_2uStream;
	SStreamInfo m_indexStream;

	uint32 m_nVertexCount = 0;
	uint32 m_nIndexCount = 0;
};

struct SAtlasBakeInformation
{
	std::vector<SAtlasGeometry>m_atlasGeometries;
	_smart_ptr<CTexture> m_pPosTex;
	_smart_ptr<CTexture> m_pFaceNormalTex;
	_smart_ptr<CTexture> m_pShadingNormalTex;
};

class CLightMapGBufferGenerator
{
public:
	void Init();
	void GenerateLightMapGBuffer(std::vector<SAtlasBakeInformation>& atlasBakeInfomation);
	void ReleaseResource();
private:
	void GenerateGraphicsPSO();
	

	CDeviceGraphicsPSOPtr m_graphicsPSO;
	CDeviceResourceLayoutPtr m_pResourceLayout;

	std::vector<SAtlasBakeInformation> m_atlasBakeInformation;
	
	CPrimitiveRenderPass         m_lightMapGBufferPass;

	bool bPsoGenerated = false;
};

