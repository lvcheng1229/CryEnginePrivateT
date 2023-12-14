#pragma once

#include "../Common/Defs.h"
#include <CryRenderer/IGIBaker.h>

class TMP_RENDER_API CGIbaker : public IGIBaker
{
public:
	virtual void AddDirectionalLight() override;
	virtual void AddPointLight() override;

	virtual void AddMesh(SGIMeshData& inputMeshData) override;

	virtual void Bake() override;
};