#include "RayTracingRenderPass.h"

CRayTracingRenderPass::CRayTracingRenderPass(CGraphicsPipeline* pGraphicsPipeline)
	: m_dirtyMask(eDirty_All)
	, m_bCompiled(false)
{
}

void CRayTracingRenderPass::SetTechnique(CShader* pShader, const CCryNameTSCRC& techName, uint64 rtMask)
{
	if (m_pShader != pShader)
	{
		m_dirtyMask |= (eDirty_Technique);
	}

	if (m_techniqueName != techName)
	{
		m_dirtyMask |= (eDirty_Technique);
	}

	if (m_rtMask != rtMask)
	{
		m_dirtyMask |= (eDirty_Technique);
	}

	m_pShader = pShader;
	m_techniqueName = techName;
	m_rtMask = rtMask;

	if (m_pPipelineState && ((!m_pPipelineState->IsValid()) || m_currentPsoUpdateCount != m_pPipelineState->GetUpdateCount()))
	{
		m_dirtyMask |= eDirty_Technique;
	}
}

void CRayTracingRenderPass::PrepareResourcesForUse(CDeviceCommandListRef RESTRICT_REFERENCE commandList)
{
	Compile();
}

CRayTracingRenderPass::EDirtyFlags CRayTracingRenderPass::Compile()
{
	EDirtyFlags dirtyMask = m_dirtyMask;

	if ((dirtyMask != eDirty_None) || (m_currentPsoUpdateCount != m_pPipelineState->GetUpdateCount()))
	{
		EDirtyFlags revertMask = dirtyMask;
		if (dirtyMask & (eDirty_Technique | eDirty_ResourceLayout))
		{
			int bindSlot = 0;
			SDeviceResourceLayoutDesc resourceLayoutDesc;
			resourceLayoutDesc.SetResourceSet(bindSlot++, m_resourceDesc);
			m_pResourceLayout = GetDeviceObjectFactory().CreateResourceLayout(resourceLayoutDesc);

			if (!m_pResourceLayout)
				return (EDirtyFlags)(m_dirtyMask |= revertMask);
		}

		if (dirtyMask & (eDirty_Technique))
		{
			CDeviceRayTracingPSODesc psoDesc(m_pResourceLayout, m_pShader, m_techniqueName, m_rtMask, 0);
			m_pPipelineState = GetDeviceObjectFactory().CreateRayTracingPSO(psoDesc);
		}
	}

	return dirtyMask;
}
