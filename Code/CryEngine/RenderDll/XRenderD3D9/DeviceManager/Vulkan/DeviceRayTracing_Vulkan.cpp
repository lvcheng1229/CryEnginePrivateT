#include "..\DeviceObjects.h"
#include "DeviceRayTracing_Vulkan.h"

#include "Vulkan\API\VKExtensions.hpp"

VkDeviceAddress InputStreamGetBufferDeviceAddress(const VkDevice* pVkDevice, const CDeviceInputStream* deviceInputStreaming)
{
	buffer_size_t offset;
	CBufferResource* const pActualBuffer = gcpRendD3D.m_DevBufMan.GetD3D(((SStreamInfo*)deviceInputStreaming)->hStream, &offset);
	VkBufferDeviceAddressInfo bufferDeviceAddressInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferDeviceAddressInfo.buffer = pActualBuffer->GetHandle();
	return vkGetBufferDeviceAddress(*pVkDevice, &bufferDeviceAddressInfo) + offset;
}

static VkFormat GetVkVertexFormat(InputLayoutHandle vertexFormat)
{
	switch (vertexFormat)
	{
	case EDefaultInputLayouts::P3F:
		return VK_FORMAT_R32G32B32_SFLOAT;
	}
	CRY_ASSERT(false);
	return VK_FORMAT_UNDEFINED;
}

static void AddAccelerationStructureBuildBarrier(VkCommandBuffer CommandBuffer)
{
	VkMemoryBarrier Barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	Barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	Barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &Barrier, 0, nullptr, 0, nullptr);
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

		VkAccelerationStructureGeometryKHR accelerationStructureGeometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };

		if (rayTracingGeometryTriangle.bForceOpaque)
		{
			accelerationStructureGeometry.flags |= VK_GEOMETRY_OPAQUE_BIT_KHR;
		}

		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;

		accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		accelerationStructureGeometry.geometry.triangles.vertexFormat = GetVkVertexFormat(rayTracingGeometryTriangle.m_sTriangVertexleInfo.m_hVertexFormat);
		accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress = vertexDeviceAddress;
		accelerationStructureGeometry.geometry.triangles.vertexStride = ((SStreamInfo*)rayTracingGeometryTriangle.m_sTriangVertexleInfo.m_sVertexStreaming)->nStride;
		accelerationStructureGeometry.geometry.triangles.indexData.deviceAddress = indexDeviceAddress;

		if (sSTriangleIndexInfo.m_sIndexStreaming)
		{
			accelerationStructureGeometry.geometry.triangles.indexType = ((SStreamInfo*)sSTriangleIndexInfo.m_sIndexStreaming)->nStride == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
		}
		else
		{
			CRY_ASSERT(false);
		}

		outvkRtBLASBuildInfo.m_vkRtGeometryTrianglesInfo.push_back(accelerationStructureGeometry);

		VkAccelerationStructureBuildRangeInfoKHR RangeInfo = {};
		RangeInfo.primitiveCount = rayTracingGeometryTriangle.bEnable ? rayTracingGeometryTriangle.m_sRangeInfo.m_nPrimitiveCount : 0;
		RangeInfo.primitiveOffset = rayTracingGeometryTriangle.m_sRangeInfo.m_nPrimitiveOffset;
		RangeInfo.firstVertex = rayTracingGeometryTriangle.m_sRangeInfo.m_nFirstVertex;
		outvkRtBLASBuildInfo.m_vkAsBuildRangeInfo.push_back(RangeInfo);

		maxPrimitiveCounts.push_back(rayTracingGeometryTriangle.m_sTriangVertexleInfo.m_nMaxVertex);
	}

	outvkRtBLASBuildInfo.m_vkAsBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	outvkRtBLASBuildInfo.m_vkAsBuildGeometryInfo.flags = uint32(eBuildFlag & EBuildAccelerationStructureFlag::eBuild_PreferBuild) != (0) ? VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	outvkRtBLASBuildInfo.m_vkAsBuildGeometryInfo.mode = uint32(eBuildMode & EBuildAccelerationStructureMode::eBuild) != 0 ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	outvkRtBLASBuildInfo.m_vkAsBuildGeometryInfo.geometryCount = outvkRtBLASBuildInfo.m_vkRtGeometryTrianglesInfo.size();
	outvkRtBLASBuildInfo.m_vkAsBuildGeometryInfo.pGeometries = outvkRtBLASBuildInfo.m_vkRtGeometryTrianglesInfo.data();

	Extensions::KHR_acceleration_structure::vkGetAccelerationStructureBuildSizesKHR(*pVkDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &outvkRtBLASBuildInfo.m_vkAsBuildGeometryInfo, maxPrimitiveCounts.data(), &outvkRtBLASBuildInfo.m_vkAsBuildSizeInfo);
}

SRayTracingAccelerationStructSize CDeviceObjectFactory::GetRayTracingBottomLevelASSizeImpl(const SRayTracingBottomLevelASCreateInfo& rtBottomLevelCreateInfo)
{
	SVulkanRayTracingBLASBuildInfo vkRtBLASBuildInfo;
	GetBottomLevelAccelerationStructureBuildInfo(
		&GetDevice()->GetVkDevice(),
		rtBottomLevelCreateInfo.m_rtGeometryTriangles,
		rtBottomLevelCreateInfo.m_sSTriangleIndexInfo,
		rtBottomLevelCreateInfo.m_eBuildFlag, EBuildAccelerationStructureMode::eBuild, vkRtBLASBuildInfo);

	uint32 nAccelerationStructureAlignment = 256;
	uint32 nScratchAlignment = std::max<uint32>(nAccelerationStructureAlignment, GetDevice()->GetVulkanDeviceExtensionProperties().m_vkAccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment);

	SRayTracingAccelerationStructSize rtAccelerationStructSize;
	rtAccelerationStructSize.m_nAccelerationStructureSize = alignedValue(vkRtBLASBuildInfo.m_vkAsBuildSizeInfo.accelerationStructureSize, nAccelerationStructureAlignment);
	rtAccelerationStructSize.m_nBuildScratchSize = alignedValue(vkRtBLASBuildInfo.m_vkAsBuildSizeInfo.buildScratchSize, nScratchAlignment);
	rtAccelerationStructSize.m_nUpdateScratchSize = alignedValue(vkRtBLASBuildInfo.m_vkAsBuildSizeInfo.updateScratchSize, nScratchAlignment);
	return rtAccelerationStructSize;
}


CVulkanRayTracingBottomLevelAccelerationStructure::CVulkanRayTracingBottomLevelAccelerationStructure(const SRayTracingBottomLevelASCreateInfo& rtBottomLevelCreateInfo, CDevice* pDevice)
	:CRayTracingBottomLevelAccelerationStructure(rtBottomLevelCreateInfo)
	, m_pDevice(pDevice)
{
	m_sSizeInfo = GetDeviceObjectFactory().GetRayTracingBottomLevelASSize(rtBottomLevelCreateInfo);

	m_accelerationStructureBuffer.Create(1u, static_cast<uint32>(m_sSizeInfo.m_nAccelerationStructureSize), DXGI_FORMAT_UNKNOWN, CDeviceObjectFactory::USAGE_ACCELERATION_STRUCTURE, nullptr);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	accelerationStructureCreateInfo.buffer = m_accelerationStructureBuffer.GetDevBuffer()->GetBuffer()->GetHandle();
	accelerationStructureCreateInfo.offset = 0;
	accelerationStructureCreateInfo.size = m_sSizeInfo.m_nAccelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

	Extensions::KHR_acceleration_structure::vkCreateAccelerationStructureKHR(m_pDevice->GetVkDevice(), &accelerationStructureCreateInfo, nullptr, &accelerationStructureHandle);

	VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
	deviceAddressInfo.accelerationStructure = accelerationStructureHandle;
	accelerationStructureDeviceAddress = Extensions::KHR_acceleration_structure::vkGetAccelerationStructureDeviceAddressKHR(m_pDevice->GetVkDevice(), &deviceAddressInfo);
}

CRayTracingBottomLevelAccelerationStructurePtr CDeviceObjectFactory::CreateRayTracingBottomLevelASImpl(const SRayTracingBottomLevelASCreateInfo& rtBottomLevelCreateInfo)
{
	return std::make_shared<CVulkanRayTracingBottomLevelAccelerationStructure>(rtBottomLevelCreateInfo, GetDevice());
}

void CDeviceGraphicsCommandInterfaceImpl::BuildRayTracingBottomLevelASsImpl(std::vector<CRayTracingBottomLevelAccelerationStructurePtr>& rtBottomLevelASPtrs)
{
	uint64 nTotalScratchBufferSize = 0;
	for (auto& blas : rtBottomLevelASPtrs)
	{
		nTotalScratchBufferSize += blas->m_sSizeInfo.m_nBuildScratchSize;
	}

	CGpuBuffer scratchBuffer;
	scratchBuffer.Create(1u, static_cast<uint32>(nTotalScratchBufferSize), DXGI_FORMAT_UNKNOWN, CDeviceObjectFactory::USAGE_STRUCTURED, nullptr);

	std::vector<SVulkanRayTracingBLASBuildInfo> tempBuildInfos;
	std::vector<VkAccelerationStructureBuildGeometryInfoKHR> buildGeometryInfos;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos;

	uint64 nScratchBufferOffset = 0;
	for (auto & blas : rtBottomLevelASPtrs)
	{
		tempBuildInfos.push_back(SVulkanRayTracingBLASBuildInfo());
		SVulkanRayTracingBLASBuildInfo& vkRtBLASBuildInfo = tempBuildInfos[tempBuildInfos.size() - 1];

		GetBottomLevelAccelerationStructureBuildInfo(
			&GetDevice()->GetVkDevice(),
			blas->m_sRtBlASCreateInfo.m_rtGeometryTriangles,
			blas->m_sRtBlASCreateInfo.m_sSTriangleIndexInfo,
			blas->m_sRtBlASCreateInfo.m_eBuildFlag, EBuildAccelerationStructureMode::eBuild, vkRtBLASBuildInfo);

		VkBufferDeviceAddressInfo bufferDeviceAddressInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
		bufferDeviceAddressInfo.buffer = scratchBuffer.GetDevBuffer()->GetBuffer()->GetHandle();
		VkDeviceAddress scratchBufferAddress = vkGetBufferDeviceAddress(GetDevice()->GetVkDevice(), &bufferDeviceAddressInfo) + nScratchBufferOffset;
		nScratchBufferOffset += blas->m_sSizeInfo.m_nBuildScratchSize;

		CVulkanRayTracingBottomLevelAccelerationStructure* vkRtBLAS = static_cast<CVulkanRayTracingBottomLevelAccelerationStructure*>(blas.get());
		vkRtBLASBuildInfo.m_vkAsBuildGeometryInfo.dstAccelerationStructure = vkRtBLAS->accelerationStructureHandle;
		vkRtBLASBuildInfo.m_vkAsBuildGeometryInfo.srcAccelerationStructure = nullptr;
		vkRtBLASBuildInfo.m_vkAsBuildGeometryInfo.scratchData.deviceAddress = scratchBufferAddress;
		
		buildGeometryInfos.push_back(vkRtBLASBuildInfo.m_vkAsBuildGeometryInfo);
		buildRangeInfos.push_back(vkRtBLASBuildInfo.m_vkAsBuildRangeInfo.data());
	}

	VkCommandBuffer cmdBuffer = GetVKCommandList()->GetVkCommandList();
	Extensions::KHR_acceleration_structure::vkCmdBuildAccelerationStructuresKHR(cmdBuffer, rtBottomLevelASPtrs.size(), buildGeometryInfos.data(), buildRangeInfos.data());
	
	AddAccelerationStructureBuildBarrier(cmdBuffer);
	GetVKCommandList()->Submit();

	//todo@ ->reset and ->begin submit at end of pass
	//GetVKCommandList()->Reset();
	//GetVKCommandList()->Begin();
}


CRayTracingTopLevelAccelerationStructurePtr CDeviceObjectFactory::CreateRayTracingTopLevelASImpl(const SRayTracingTopLevelASCreateInfo& rtBottomLevelCreateInfo)
{
	return CRayTracingTopLevelAccelerationStructurePtr();
}





