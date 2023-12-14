#include "GIBaker/GIBaker.h"

static CGIbaker* pGIBaker = nullptr;

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

void CGIbaker::AddMesh(SGIMeshData& inputMeshData)
{

}

void CGIbaker::Bake()
{

}
