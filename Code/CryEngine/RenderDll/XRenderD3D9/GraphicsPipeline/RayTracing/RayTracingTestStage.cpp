#include "RayTracingTestStage.h"

void CRayTracingTestStage::CreateVbIb()
{
	// Create vertex buffer and index buffer
	m_RtVertices.push_back(SRtVertex{ Vec3(1, -1, 0) });
	m_RtVertices.push_back(SRtVertex{ Vec3(1,  1, 0) });
	m_RtVertices.push_back(SRtVertex{ Vec3(-1,  1, 0) });

	m_RtIndices.push_back(0);
	m_RtIndices.push_back(1);
	m_RtIndices.push_back(2);

	m_pVB = gcpRendD3D->m_DevBufMan.Create(BBT_VERTEX_BUFFER, BU_STATIC, m_RtVertices.size() * sizeof(SRtVertex));
	m_pIB = gcpRendD3D->m_DevBufMan.Create(BBT_INDEX_BUFFER, BU_STATIC, m_RtIndices.size() * sizeof(SRtIndex));

	gcpRendD3D->m_DevBufMan.UpdateBuffer(m_pVB, &m_RtVertices[0], m_RtVertices.size() * sizeof(SRtVertex));
	gcpRendD3D->m_DevBufMan.UpdateBuffer(m_pIB, &m_RtIndices[0], m_RtIndices.size() * sizeof(SRtIndex));

	SStreamInfo vertexStreamInfo;
	vertexStreamInfo.hStream = m_pVB;
	vertexStreamInfo.nStride = sizeof(SRtVertex);
	vertexStreamInfo.nSlot = 0;
	m_pVertexInputSet = GetDeviceObjectFactory().CreateVertexStreamSet(1, &vertexStreamInfo);

	SStreamInfo indexStreamInfo;
	indexStreamInfo.hStream = m_pIB;
	indexStreamInfo.nStride = sizeof(SRtIndex);
	indexStreamInfo.nSlot = 0;
	m_pIndexInputSet = GetDeviceObjectFactory().CreateIndexStreamSet(&indexStreamInfo);
}


void CRayTracingTestStage::Init()
{
	CreateVbIb();

	CDeviceGraphicsCommandInterface* pCommandInterface = GetDeviceObjectFactory().GetCoreCommandList().GetGraphicsInterface();

	CRayTracingBottomLevelAccelerationStructurePtr m_pRtBottomLevelAS = GetDeviceObjectFactory().CreateRayTracingBottomLevelAS();
	pCommandInterface->BuildRayTracingBottomLevelAS(m_pRtBottomLevelAS);
	
}

void CRayTracingTestStage::Execute()
{
}

