//TanGram:VSM
#pragma once
#include "DevBuffer.h"

class CObjGPUManager :IObjGPUManager
{
public:
	CObjGPUManager();
	CGpuBuffer m_primitiveBuffer;
	int32 AA;
};