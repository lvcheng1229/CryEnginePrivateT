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

void CDeviceGraphicsCommandInterface::BuildRayTracingBottomLevelASs(std::vector<CRayTracingBottomLevelAccelerationStructurePtr>& rtBottomLevelASPtrs)
{
	BuildRayTracingBottomLevelASsImpl(rtBottomLevelASPtrs);
}