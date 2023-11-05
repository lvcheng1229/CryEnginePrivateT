#pragma once
#include <memory>
#include "../../DeviceManager/DeviceResources.h"
using namespace NCryVulkan;

struct SVulkanRayTracingBLASBuildInfo
{
	SVulkanRayTracingBLASBuildInfo()
	{
		m_vkAsBuildGeometryInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		m_vkAsBuildSizeInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
	}
	std::vector<VkAccelerationStructureGeometryKHR>m_vkRtGeometryTrianglesInfo;
	
	std::vector<VkAccelerationStructureBuildRangeInfoKHR>m_vkAsBuildRangeInfo; // Build range info
	VkAccelerationStructureBuildGeometryInfoKHR m_vkAsBuildGeometryInfo; // Build geometry info
	VkAccelerationStructureBuildSizesInfoKHR m_vkAsBuildSizeInfo; // Build size info
};

class CVulkanRayTracingBottomLevelAccelerationStructure : public CRayTracingBottomLevelAccelerationStructure
{
public:
	CVulkanRayTracingBottomLevelAccelerationStructure(const SRayTracingBottomLevelASCreateInfo& rtBottomLevelCreateInfo,CDevice* pDevice);

	VkAccelerationStructureKHR accelerationStructureHandle = nullptr;
	VkDeviceAddress accelerationStructureDeviceAddress;

	CGpuBuffer m_accelerationStructureBuffer;
private:
	CDevice* const  m_pDevice;
};