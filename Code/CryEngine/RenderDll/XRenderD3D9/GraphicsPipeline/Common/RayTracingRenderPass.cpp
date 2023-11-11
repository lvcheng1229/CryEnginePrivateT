//#include "RayTracingRenderPass.h"
//
//CRayTracingRenderPass::CRayTracingRenderPass(CGraphicsPipeline* pGraphicsPipeline)
//{
//}
//
//void CRayTracingRenderPass::SetTechnique(CShader* pShader, const CCryNameTSCRC& techName, uint64 rtMask)
//{
//	if (m_pShader != pShader)
//	{
//		m_dirtyMask |= (eDirty_Technique);
//	}
//
//	if (m_techniqueName != techName)
//	{
//		m_dirtyMask |= (eDirty_Technique);
//	}
//
//	if (m_rtMask != rtMask)
//	{
//		m_dirtyMask |= (eDirty_Technique);
//	}
//
//	m_pShader = pShader;
//	m_techniqueName = techName;
//	m_rtMask = rtMask;
//
//	//if (m_pPipelineState && (!m_pPipelineState->IsValid()) || m_currentPsoUpdateCount != m_pPipelineState->GetUpdateCount())
//	//{
//	//	m_dirtyMask |= eDirty_Technique;
//	//}
//}
//
//CRayTracingRenderPass::EDirtyFlags CRayTracingRenderPass::Compile()
//{
//	EDirtyFlags dirtyMask = m_dirtyMask;
//
//	//if ((dirtyMask != eDirty_None) || (m_currentPsoUpdateCount != m_pPipelineState->GetUpdateCount()))
//	//{
//	//	if (dirtyMask & (eDirty_Technique))
//	//	{
//	//
//	//	}
//	//}
//
//	return dirtyMask;
//}
