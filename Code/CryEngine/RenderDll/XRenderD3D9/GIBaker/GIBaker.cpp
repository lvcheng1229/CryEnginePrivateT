#include "GIBaker/GIBaker.h"
#include "XRenderD3D9/DriverD3D.h"

CGIbaker* pGIBaker = nullptr;

IGIBaker* CD3D9Renderer::GetIGIBaker()
{
	if (!pGIBaker)
	{
		pGIBaker = new CGIbaker();
	}

	return pGIBaker;
}

void CGIbaker::AddDirectionalLight()
{

}

void CGIbaker::AddPointLight()
{

}

void CGIbaker::AddMesh(IStatObj* inputStatObj, Matrix34 worldTM, Vec2i atlasSize)
{
	m_giMeshArray.push_back(SGIMeshDescription());

	IIndexedMesh* pIMesh = inputStatObj->GetIndexedMesh();
	
	uint32 indexCount = pIMesh->GetIndexCount();
	uint32 vertexCount = pIMesh->GetVertexCount();
	uint32 lightMapUVCount = pIMesh->GetLightMapUVCount();
	assert(vertexCount == lightMapUVCount);

	SGIMeshDescription& giMeshDescription = m_giMeshArray.back();

	giMeshDescription.m_positions.resize(vertexCount);
	giMeshDescription.m_normals.resize(vertexCount);
	giMeshDescription.m_lightMapUVs.resize(vertexCount);
	giMeshDescription.m_indices.resize(indexCount);

	IIndexedMesh::SMeshDescription meshDescription;
	pIMesh->GetMeshDescription(meshDescription);
	assert(meshDescription.m_nVertCount = vertexCount);
	assert(meshDescription.m_nLightMapUVCount = vertexCount);
	assert(meshDescription.m_nIndexCount = indexCount);

	memcpy(giMeshDescription.m_positions.data(), meshDescription.m_pVerts, vertexCount * sizeof(Vec3));
	memcpy(giMeshDescription.m_normals.data(), meshDescription.m_pNorms, vertexCount * sizeof(SMeshNormal));
	memcpy(giMeshDescription.m_lightMapUVs.data(), meshDescription.m_pLightMapUV, vertexCount * sizeof(SMeshTexCoord));
	memcpy(giMeshDescription.m_indices.data(), meshDescription.m_pIndices, indexCount * sizeof(vtx_idx));

	giMeshDescription.m_vertexCount = vertexCount;
	giMeshDescription.m_indexCount  = indexCount;

	giMeshDescription.m_worldTM = worldTM;
}

void CGIbaker::GenerateBufferHandle()
{
	for (uint32 index = 0; index < m_giMeshArray.size(); index++)
	{
		SGIMeshDescription& giMeshDesc = m_giMeshArray[index];

		buffer_handle_t pPosBuffer = gcpRendD3D->m_DevBufMan.Create(BBT_VERTEX_BUFFER, BU_STATIC, giMeshDesc.m_vertexCount * sizeof(Vec3));
		buffer_handle_t pNormalBuffer = gcpRendD3D->m_DevBufMan.Create(BBT_VERTEX_BUFFER, BU_STATIC, giMeshDesc.m_vertexCount * sizeof(SMeshNormal));
		buffer_handle_t p2UBuffer = gcpRendD3D->m_DevBufMan.Create(BBT_VERTEX_BUFFER, BU_STATIC, giMeshDesc.m_vertexCount * sizeof(SMeshTexCoord));
		buffer_handle_t pIndexBuffer = gcpRendD3D->m_DevBufMan.Create(BBT_INDEX_BUFFER, BU_STATIC, giMeshDesc.m_indexCount * sizeof(vtx_idx));

		gcpRendD3D->m_DevBufMan.UpdateBuffer(pPosBuffer, giMeshDesc.m_positions.data(), giMeshDesc.m_vertexCount * sizeof(Vec3));
		gcpRendD3D->m_DevBufMan.UpdateBuffer(pNormalBuffer, giMeshDesc.m_normals.data(), giMeshDesc.m_vertexCount * sizeof(SMeshNormal));
		gcpRendD3D->m_DevBufMan.UpdateBuffer(p2UBuffer, giMeshDesc.m_lightMapUVs.data(), giMeshDesc.m_vertexCount * sizeof(SMeshTexCoord));
		gcpRendD3D->m_DevBufMan.UpdateBuffer(pIndexBuffer, giMeshDesc.m_indices.data(), giMeshDesc.m_indexCount * sizeof(vtx_idx));

		SStreamInfo posStreamInfo;
		posStreamInfo.hStream = pPosBuffer;
		posStreamInfo.nStride = sizeof(Vec3);
		posStreamInfo.nSlot = EStreamIDs::VSF_VERTEX_VELOCITY;
		
		SStreamInfo normStreamInfo;
		normStreamInfo.hStream = pNormalBuffer;
		normStreamInfo.nStride = sizeof(SMeshNormal);
		normStreamInfo.nSlot = EStreamIDs::VSF_NORMALS;

		SStreamInfo tuStreamInfo;
		tuStreamInfo.hStream = p2UBuffer;
		tuStreamInfo.nStride = sizeof(SMeshTexCoord);
		tuStreamInfo.nSlot = EStreamIDs::VSF_LIGHTMAPUV;

		SStreamInfo indexStreamInfo;
		indexStreamInfo.hStream = pIndexBuffer;
		indexStreamInfo.nStride = sizeof(vtx_idx);
		indexStreamInfo.nSlot = 0;

		m_atlasGeometries.push_back(SAtlasGeometry{ posStreamInfo ,normStreamInfo ,tuStreamInfo ,indexStreamInfo ,giMeshDesc.m_vertexCount,giMeshDesc.m_indexCount });
	}

	m_atlasBakeInfomation.push_back(SAtlasBakeInformation{ m_atlasGeometries });

	bInit = true;
}

void CGIbaker::GenerateLightMapGBuffer()
{
	m_lightMapGBufferGenerator.GenerateLightMapGBuffer(m_atlasBakeInfomation);
}

void CGIbaker::RealseResource()
{
	m_lightMapGBufferGenerator.ReleaseResource();

	//for (uint32 index = 0; index < m_atlasBakeInfomation.size(); index++)
	//{
	//	std::vector<SAtlasGeometry>& atlasGeo = m_atlasBakeInfomation[index].m_atlasGeometries;
	//	for (uint32 indexGeo = 0; indexGeo < atlasGeo.size(); indexGeo++)
	//	{
	//		if (atlasGeo[indexGeo].m_posStream.hStream != ~0u)
	//		{
	//			gcpRendD3D->m_DevBufMan.Destroy(atlasGeo[indexGeo].m_posStream.hStream);
	//		}
	//		
	//		if (atlasGeo[indexGeo].m_normStream.hStream != ~0u)
	//		{
	//			gcpRendD3D->m_DevBufMan.Destroy(atlasGeo[indexGeo].m_normStream.hStream);
	//		}
	//
	//		if (atlasGeo[indexGeo].m_2uStream.hStream != ~0u)
	//		{
	//			gcpRendD3D->m_DevBufMan.Destroy(atlasGeo[indexGeo].m_2uStream.hStream);
	//		}
	//
	//		if (atlasGeo[indexGeo].m_indexStream.hStream != ~0u)
	//		{
	//			gcpRendD3D->m_DevBufMan.Destroy(atlasGeo[indexGeo].m_indexStream.hStream);
	//		}
	//	}
	//}
	//m_atlasBakeInfomation.clear();
	//m_atlasBakeInfomation.resize(0);
}

void CGIbaker::Bake()
{
	CD3D9Renderer* d3dRender = static_cast<CD3D9Renderer*>(gRenDev);
	//d3dRender->BeginNSightCapture();
	d3dRender->m_pBakeFunctionCallBack = ([=]
		{
			if (!this->bInit)
			{
				this->GenerateBufferHandle();
			}
			
			this->GenerateLightMapGBuffer();
			this->RealseResource();
		});
}
