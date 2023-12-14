#pragma once

#include "../CryMath/Cry_Vector3.h"
#include "../Cry3DEngine/IIndexedMesh.h"
#include <vector>

struct SGIMeshData
{
	std::vector<Vec3> m_position;
	std::vector<Vec3> m_normal;
	std::vector<SMeshTexCoord> m_lightMapUV;
};

struct IGIBaker
{
public:
	virtual void AddDirectionalLight() = 0;
	virtual void AddPointLight() = 0;

	virtual void AddMesh(SGIMeshData& inputMeshData) = 0;

	virtual void Bake() = 0;
};