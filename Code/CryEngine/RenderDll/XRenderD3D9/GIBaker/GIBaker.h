#pragma once
#include <CryRenderer/IGIBaker.h>
#include "LightMapGBufferPass.h"
struct SGIMeshDescription
{
	std::vector<Vec3>m_positions;
	std::vector<Vec3>m_normals;
	std::vector<SMeshTexCoord>m_lightMapUVs;
	std::vector<vtx_idx>m_indices;

	Matrix34 m_worldTM;

	Vec2i m_atlasSize;

	uint32 m_vertexCount;
	uint32 m_indexCount;
};

struct SGIMeshBufferHandles
{
	buffer_handle_t m_pPos;
	buffer_handle_t m_pNorm;
	buffer_handle_t m_p2U;
	buffer_handle_t m_pIB;
};

class CGIbaker : public IGIBaker
{
public:

	bool bInit = false;

	CGIbaker() {};

	virtual void AddDirectionalLight() override;
	virtual void AddPointLight() override;

	virtual void AddMesh(IStatObj* inputStatObj, Matrix34 worldTM, Vec2i atlasSize) override;

	virtual void Bake() override;

	void GenerateBufferHandle();
	void GenerateLightMapGBuffer();
	void RealseResource();

private:
	std::vector<SGIMeshDescription> m_giMeshArray;
	
	//temporary
	std::vector<SAtlasGeometry>m_atlasGeometries;
	std::vector<SAtlasBakeInformation> m_atlasBakeInfomation;

	CLightMapGBufferGenerator m_lightMapGBufferGenerator;
};