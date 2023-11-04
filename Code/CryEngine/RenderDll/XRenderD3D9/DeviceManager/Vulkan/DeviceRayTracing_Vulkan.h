#pragma once
#include <memory>
#include "../../DeviceManager/DeviceResources.h"
using namespace NCryVulkan;

struct SVulkanRayTracingBLASBuildInfo
{
	std::vector<VkAccelerationStructureGeometryKHR>m_vkRtGeometryTrianglesInfo;
	
	std::vector<VkAccelerationStructureBuildRangeInfoKHR>m_vkAsBuildRangeInfo; // Build range info
	VkAccelerationStructureBuildGeometryInfoKHR m_vkAsBuildGeometryInfo; // Build geometry info
	VkAccelerationStructureBuildSizesInfoKHR m_vkAsBuildSizeInfo; // Build size info
};

class CVulkanRayTracingBottomLevelAccelerationStructure : public CRayTracingBottomLevelAccelerationStructure
{
public:
	CVulkanRayTracingBottomLevelAccelerationStructure(const SRayTracingBottomLevelASCreateInfo& rtBottomLevelCreateInfo,CDevice* pDevice);
private:
	CDevice* const  m_pDevice;
};