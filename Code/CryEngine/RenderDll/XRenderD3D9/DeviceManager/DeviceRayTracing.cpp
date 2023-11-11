#include "StdAfx.h"
#include "xxhash.h"
#include "DeviceObjects.h"
#include "DeviceCommandListCommon.h"
#include "Common/ReverseDepth.h"
#include "Common/Textures/TextureHelpers.h"
#include "../GraphicsPipeline/Common/GraphicsPipelineStateSet.h"

CRayTracingBottomLevelAccelerationStructurePtr CDeviceObjectFactory::CreateRayTracingBottomLevelAS(const SRayTracingBottomLevelASCreateInfo& rtBottomLevelCreateInfo)
{
	return CreateRayTracingBottomLevelASImpl(rtBottomLevelCreateInfo);
}
CRayTracingTopLevelAccelerationStructurePtr CDeviceObjectFactory::CreateRayTracingTopLevelAS(const SRayTracingTopLevelASCreateInfo& rtTopLevelCreateInfo)
{
	return CreateRayTracingTopLevelASImpl(rtTopLevelCreateInfo);
}
SRayTracingAccelerationStructSize CDeviceObjectFactory::GetRayTracingBottomLevelASSize(const SRayTracingBottomLevelASCreateInfo& rtBottomLevelCreateInfo)
{
	return GetRayTracingBottomLevelASSizeImpl(rtBottomLevelCreateInfo);
}

SRayTracingAccelerationStructSize CDeviceObjectFactory::GetRayTracingTopLevelASSize(const SRayTracingTopLevelASCreateInfo& rtTopLevelCreateInfo)
{
	return GetRayTracingTopLevelASSizeImpl(rtTopLevelCreateInfo);
}

void CDeviceGraphicsCommandInterface::BuildRayTracingBottomLevelASs(std::vector<CRayTracingBottomLevelAccelerationStructurePtr>& rtBottomLevelASPtrs)
{
	BuildRayTracingBottomLevelASsImpl(rtBottomLevelASPtrs);
}

void CDeviceGraphicsCommandInterface::BuildRayTracingTopLevelAS(CRayTracingTopLevelAccelerationStructurePtr rtTopLevelASPtr, CGpuBuffer* instanceBuffer, uint32 offset)
{
	BuildRayTracingTopLevelASImpl(rtTopLevelASPtr, instanceBuffer, offset);
}

//CDeviceRayTracingPSODesc::CDeviceRayTracingPSODesc(CShader* pShader, const CCryNameTSCRC& technique, uint64 rtFlags, uint32 mdFlags)
//{
//	memset(this, 0, sizeof(CDeviceRayTracingPSODesc));
//}




