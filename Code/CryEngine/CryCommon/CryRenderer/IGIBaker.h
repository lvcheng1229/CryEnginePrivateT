#pragma once

#include "../CryMath/Cry_Vector3.h"
#include "../Cry3DEngine/IIndexedMesh.h"
#include <vector>

struct IGIBaker
{
public:
	virtual void AddDirectionalLight() = 0;
	virtual void AddPointLight() = 0;
	virtual void AddMesh(IStatObj* inputStatObj, Matrix34 worldTM, Vec2i atlasSize) = 0;
	virtual void Bake() = 0;
};