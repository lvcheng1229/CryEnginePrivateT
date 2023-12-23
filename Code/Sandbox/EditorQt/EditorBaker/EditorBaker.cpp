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
	static bool bAdded = false;
	
	IGIBaker* pGIBaker = gEnv->pRenderer->GetIGIBaker();

	if (!bAdded)
	{
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
					if (!objects[i]->IsKindOf(RUNTIME_CLASS(CBrushObject)))
					{
						continue;
					}

					CBrushObject* pObj = (CBrushObject*)(objects[i]);
					IStatObj* pStatObj = pObj->GetIStatObj();
					IIndexedMesh* pIMesh = pStatObj->GetIndexedMesh();
					if (pIMesh)
					{
						pGIBaker->AddMesh(pStatObj, pObj->GetWorldTM(),Vec2i(0,0));
						addedObjects.insert(objects[i]);
					}
				}
			}
		}

		bAdded = true;
	}
	

	pGIBaker->Bake();
}