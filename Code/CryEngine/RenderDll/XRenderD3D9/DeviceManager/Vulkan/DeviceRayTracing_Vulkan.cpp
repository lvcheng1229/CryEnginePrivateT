#include "..\DeviceObjects.h"
#include "DeviceRayTracing_Vulkan.h"

VkDeviceAddress InputStreamGetBufferDeviceAddress(const VkDevice* pVkDevice, const CDeviceInputStream* deviceInputStreaming)
{
	buffer_size_t offset;
	CBufferResource* const pActualBuffer = gcpRendD3D.m_DevBufMan.GetD3D(((SStreamInfo*)deviceInputStreaming)->hStream, &offset);
	VkBufferDeviceAddressInfo bufferDeviceAddressInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferDeviceAddressInfo.buffer = pActualBuffer->GetHandle();
	return vkGetBufferDeviceAddress(*pVkDevice, &bufferDeviceAddressInfo) + offset;
}

// Get build geometry info / build range info / build size info
static void GetBottomLevelAccelerationStructureBuildInfo(
	const VkDevice* pVkDevice,
	const std::vector<SRayTracingGeometryTriangle>& rtGeometryTriangles, 
	const SRayTracingBottomLevelASCreateInfo::STriangleIndexInfo& sSTriangleIndexInfo,
	EBuildAccelerationStructureFlag eBuildFlag,
	EBuildAccelerationStructureMode eBuildMode,
	SVulkanRayTracingBLASBuildInfo& outvkRtBLASBuildInfo)
{
	VkDeviceAddress indexDeviceAddress = sSTriangleIndexInfo.m_sIndexStreaming ? InputStreamGetBufferDeviceAddress(pVkDevice, sSTriangleIndexInfo.m_sIndexStreaming) : 0;
	
	std::vector<uint32> maxPrimitiveCounts;
	for (uint32 index = 0; index < rtGeometryTriangles.size(); index++)
	{
		const SRayTracingGeometryTriangle& rayTracingGeometryTriangle = rtGeometryTriangles[index];
		VkDeviceAddress vertexDeviceAddress = InputStreamGetBufferDeviceAddress(pVkDevice, rayTracingGeometryTriangle.m_sTriangVertexleInfo.m_sVertexStreaming);
	}
	
}

SRayTracingAccelerationStructSize CDeviceObjectFactory::GetRayTracingBottomLevelASSizeImpl(const SRayTracingBottomLevelASCreateInfo& rtBottomLevelCreateInfo)
{
	SVulkanRayTracingBLASBuildInfo vkRtBLASBuildInfo;
	GetBottomLevelAccelerationStructureBuildInfo(
		&GetDevice()->GetVkDevice(),
		rtBottomLevelCreateInfo.m_rtGeometryTriangles,
		rtBottomLevelCreateInfo.m_sSTriangleIndexInfo,
		rtBottomLevelCreateInfo.m_eBuildFlag, EBuildAccelerationStructureMode::eBuild, vkRtBLASBuildInfo);

	SRayTracingAccelerationStructSize rtAccelerationStructSize;
	//rtAccelerationStructSize.m_nAccelerationStructureSize = vkRtBLASBuildInfo.m_vkAsBuildSizeInfo.accelerationStructureSize;
	return rtAccelerationStructSize;
}


CVulkanRayTracingBottomLevelAccelerationStructure::CVulkanRayTracingBottomLevelAccelerationStructure(const SRayTracingBottomLevelASCreateInfo& rtBottomLevelCreateInfo, CDevice* pDevice)
	:CRayTracingBottomLevelAccelerationStructure(rtBottomLevelCreateInfo)
	, m_pDevice(pDevice)
{

}

CRayTracingBottomLevelAccelerationStructurePtr CDeviceObjectFactory::CreateRayTracingBottomLevelASImpl(const SRayTracingBottomLevelASCreateInfo& rtBottomLevelCreateInfo)
{
	return std::make_shared<CVulkanRayTracingBottomLevelAccelerationStructure>(rtBottomLevelCreateInfo, GetDevice());
}





