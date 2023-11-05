#pragma once
#include "../Common/GraphicsPipelineStage.h"
#include "../Common/FullscreenPass.h"
#include "../Common/ComputeRenderPass.h"
#include "../Common/GraphicsPipeline.h"

class CRayTracingTestStage : public CGraphicsPipelineStage
{
public:
	static const EGraphicsPipelineStage StageID = eStage_RayTracingTest;

	CRayTracingTestStage(CGraphicsPipeline& graphicsPipeline)
		: CGraphicsPipelineStage(graphicsPipeline)
	{

	}

	void  Init()   final;
	void Execute();
	bool IsStageActive(EShaderRenderingFlags flags) const final
	{
		return true;
	}

	
	void CreateVbIb();
	void CreateAndBuildBLAS(CDeviceGraphicsCommandInterface* pCommandInterface);
	void CreateAndBuildTLAS(CDeviceGraphicsCommandInterface* pCommandInterface);

private:

	//VB IB
	struct SRtVertex
	{
		Vec3 m_Pos;
	};

	using SRtIndex = uint16;

	std::vector<SRtVertex>m_RtVertices;
	std::vector<SRtIndex>m_RtIndices;

	buffer_handle_t m_pVB;
	buffer_handle_t m_pIB;

	SStreamInfo vertexStreamInfo;
	SStreamInfo indexStreamInfo;

	const CDeviceInputStream* m_pVertexInputSet = nullptr;
	const CDeviceInputStream* m_pIndexInputSet = nullptr;

	CRayTracingBottomLevelAccelerationStructurePtr m_pRtBottomLevelAS;
	CRayTracingTopLevelAccelerationStructurePtr m_pRtTopLevelAS;
};