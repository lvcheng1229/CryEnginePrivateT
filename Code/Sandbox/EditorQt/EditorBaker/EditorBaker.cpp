#include "StdAfx.h"
#include "EditorBaker.h"
#include "GameEngine.h"

#include "Objects/EntityObject.h"
#include "Objects/Group.h"
#include "Objects/BrushObject.h"

#include <set>
#include <IObjectManager.h>
#include <CryRenderer/IRenderer.h>
#include <CryRenderer/IGIBaker.h>
#include <Cry3DEngine/IStatObj.h>

CEditorBaker::CEditorBaker()
{
}

CEditorBaker::~CEditorBaker()
{
}

void CEditorBaker::Bake(IObjectManager* pObjectManager)
{
	IGIBaker* pGIBaker = gEnv->pRenderer->GetIGIBaker();

	CBaseObjectsArray objects;
	pObjectManager->GetObjects(objects);
	
	std::set<void*> addedObjects;//will track the already added objects to never include an object twice (prefabs contain objects refered to elsewhere too)
	const int cObjectsSize = objects.size();
	
	// add meshes
	for (int i = 0; i < cObjectsSize; ++i)
	{
		if (objects[i]->GetType() == OBJTYPE_BRUSH)
		{
			if (addedObjects.find(objects[i]) == addedObjects.end())
			{
				CBrushObject* pObj = (CBrushObject*)(objects[i]);
				IStatObj* pStatObj = pObj->GetIStatObj();
				IIndexedMesh* pIMesh = pStatObj->GetIndexedMesh();
				if (pIMesh)
				{
					CMesh* pMesh = pIMesh->GetMesh();
					
					SGIMeshData giMeshData;

					int pPositionCount;
					int pNormalCount;
					int pLightMapUVCount;

					Vec3* pPosition = pMesh->GetStreamPtr<Vec3>(CMesh::POSITIONS, &pPositionCount);
					Vec3* pNormal = pMesh->GetStreamPtr<Vec3>(CMesh::NORMALS, &pNormalCount);
					SMeshTexCoord* pLightMapUV = pMesh->GetStreamPtr<SMeshTexCoord>(CMesh::LIGHTMAPUV, &pLightMapUVCount);
					
					assert(pPositionCount == pNormalCount && pNormalCount == pLightMapUVCount);

					giMeshData.m_position.resize(pPositionCount);
					giMeshData.m_normal.resize(pPositionCount);
					giMeshData.m_lightMapUV.resize(pPositionCount);

					for (int i = 0; i < pPositionCount; ++i)
					{
						giMeshData.m_position[i] = pPosition[i];
						giMeshData.m_normal[i] = pNormal[i];
						giMeshData.m_lightMapUV[i] = pLightMapUV[i];
					}

					pGIBaker->AddMesh(giMeshData);

					addedObjects.insert(objects[i]);
				}
			}
		}
	}

	

	pGIBaker->Bake();
}