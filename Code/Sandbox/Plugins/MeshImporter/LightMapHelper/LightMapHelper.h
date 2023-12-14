#pragma once

struct SLightMapResult
{
	int m_width;
	int m_height;
};

void GenerateLightMapUV(void* inputData, int nVertex, SLightMapResult* pReturnResult);
