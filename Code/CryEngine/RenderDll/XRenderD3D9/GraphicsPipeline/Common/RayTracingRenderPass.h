#pragma once
//#include "RenderPassBase.h"
////#include "Common/Shaders/CShader.h"
//
//class CRayTracingRenderPass : public CRenderPassBase
//{
//public:
//
//	enum EDirtyFlags : uint32
//	{
//		eDirty_Resources = BIT(1),
//		eDirty_Technique = BIT(2),
//
//		eDirty_None = 0,
//		eDirty_All = eDirty_Technique | eDirty_Resources,
//	};
//
//
//	CRayTracingRenderPass(CGraphicsPipeline* pGraphicsPipeline);
//
//	void SetTechnique(CShader* pShader, const CCryNameTSCRC& techName, uint64 rtMask);
//
//private:
//
//	EDirtyFlags Compile();
//
//private:
//	EDirtyFlags              m_dirtyMask;
//
//	CShader*				 m_pShader;
//	CCryNameTSCRC            m_techniqueName;
//	uint64                   m_rtMask;
//
//
//	//CDeviceRayTracingPSOPtr  m_pPipelineState;
//	int                      m_currentPsoUpdateCount;
//};
//
//DEFINE_ENUM_FLAG_OPERATORS(CRayTracingRenderPass::EDirtyFlags);