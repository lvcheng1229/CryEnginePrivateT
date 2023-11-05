// Copyright 2016-2021 Crytek GmbH / Crytek Group. All rights reserved.
#include "StdAfx.h"

#include "VKExtensions.hpp"

template <typename PreStructType, typename NextStructType>
static void AddToPNext(PreStructType& preStruct, NextStructType& nextStruct)
{
	nextStruct.pNext = (void*)preStruct.pNext;
	preStruct.pNext = (void*)&nextStruct;
}

namespace NCryVulkan { 
namespace Extensions 
{


	class CVulkanAccelerationStructureExtension :public CVulkanDeviceExtensionWithFeature
	{
	public:
		CVulkanAccelerationStructureExtension()
			:CVulkanDeviceExtensionWithFeature(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
		{}

		virtual void WritePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2KHR& PhysicalDeviceFeatures2)override
		{
			m_vkAccelerationStructureFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
			m_vkAccelerationStructureFeatures.accelerationStructure = true;
			AddToPNext(PhysicalDeviceFeatures2, m_vkAccelerationStructureFeatures);
		}

		virtual void GetPhysicalDeviceProperties(VkPhysicalDeviceProperties2KHR& PhysicalDeviceProperties2, CDevice* pDevice)override
		{
			SVulkanDeviceExtensionProperties& vkDeviceExtensionProperties = pDevice->GetVulkanDeviceExtensionProperties();
			VkPhysicalDeviceAccelerationStructurePropertiesKHR& accelerationStructure = vkDeviceExtensionProperties.m_vkAccelerationStructureProperties;
			accelerationStructure = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR };
			AddToPNext(PhysicalDeviceProperties2, accelerationStructure);
		}

		virtual void EnablePhysicalDeviceFeatures(VkDeviceCreateInfo& DeviceInfo) override
		{
			AddToPNext(DeviceInfo, m_vkAccelerationStructureFeatures);
		}
	private:
		VkPhysicalDeviceAccelerationStructureFeaturesKHR m_vkAccelerationStructureFeatures;
	};

	class CVulkanRayTracingPipelineExtension :public CVulkanDeviceExtensionWithFeature
	{
	public:
		CVulkanRayTracingPipelineExtension()
			:CVulkanDeviceExtensionWithFeature(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
		{}

		virtual void WritePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2KHR& PhysicalDeviceFeatures2)override
		{
			m_vkRtPipelineFeature = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
			AddToPNext(PhysicalDeviceFeatures2, m_vkRtPipelineFeature);
		}

		virtual void GetPhysicalDeviceProperties(VkPhysicalDeviceProperties2KHR& PhysicalDeviceProperties2, CDevice* pDevice)override
		{
			SVulkanDeviceExtensionProperties& vkDeviceExtensionProperties = pDevice->GetVulkanDeviceExtensionProperties();
			VkPhysicalDeviceRayTracingPipelinePropertiesKHR& rayTracingPipelineProperties = vkDeviceExtensionProperties.m_vkRayTracingPipelineProperties;
			rayTracingPipelineProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
			AddToPNext(PhysicalDeviceProperties2, rayTracingPipelineProperties);
		}

		virtual void EnablePhysicalDeviceFeatures(VkDeviceCreateInfo& DeviceInfo) override
		{
			AddToPNext(DeviceInfo, m_vkRtPipelineFeature);
		}
	private:
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_vkRtPipelineFeature;
	};

	class CVulkanKHRBufferDeviceAddressExtension : public CVulkanDeviceExtensionWithFeature
	{
	public:

		CVulkanKHRBufferDeviceAddressExtension()
			: CVulkanDeviceExtensionWithFeature(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) {}

		virtual void WritePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2KHR& PhysicalDeviceFeatures2)override
		{
			m_bufferDeviceAddressFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
			AddToPNext(PhysicalDeviceFeatures2, m_bufferDeviceAddressFeatures);
		}


		virtual void EnablePhysicalDeviceFeatures(VkDeviceCreateInfo& DeviceInfo) override
		{
			AddToPNext(DeviceInfo, m_bufferDeviceAddressFeatures);
		}

	private:
		VkPhysicalDeviceBufferDeviceAddressFeaturesKHR m_bufferDeviceAddressFeatures;
	};

	//https://github.com/KhronosGroup/Vulkan-Samples/blob/main/samples/extensions/descriptor_buffer_basic/descriptor_buffer_basic.cpp

	//class CVulkanDescriptorBufferExtension :public CVulkanDeviceExtensionWithFeature
	//{
	//public:
	//	CVulkanDescriptorBufferExtension()
	//		:CVulkanDeviceExtensionWithFeature(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME)
	//	{}
	//
	//	virtual void WritePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2KHR& PhysicalDeviceFeatures2)override
	//	{
	//		m_vkDescriptorBufferFeature = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT };
	//		AddToPNext(PhysicalDeviceFeatures2, m_vkDescriptorBufferFeature);
	//	}
	//
	//	virtual void GetPhysicalDeviceProperties(VkPhysicalDeviceProperties2KHR& PhysicalDeviceProperties2, CDevice* pDevice)override
	//	{
	//		SVulkanDeviceExtensionProperties& vkDeviceExtensionProperties = pDevice->GetVulkanDeviceExtensionProperties();
	//		VkPhysicalDeviceDescriptorBufferPropertiesEXT& descriptorBufferProperties = vkDeviceExtensionProperties.m_vkDescriptorBufferPropsProperties;
	//		descriptorBufferProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
	//		AddToPNext(PhysicalDeviceProperties2, descriptorBufferProperties);
	//	}
	//
	//	virtual void EnablePhysicalDeviceFeatures(VkDeviceCreateInfo& DeviceInfo) override
	//	{
	//		AddToPNext(DeviceInfo, m_vkDescriptorBufferFeature);
	//	}
	//private:
	//	VkPhysicalDeviceDescriptorBufferFeaturesEXT m_vkDescriptorBufferFeature;
	//};
	
	class CVulkanDefferedHostOperationExtension :public CVulkanDeviceExtensionWithFeature
	{
	public:
		CVulkanDefferedHostOperationExtension()
			:CVulkanDeviceExtensionWithFeature(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
		{}
	};

	class CVulkanSpirvExtension :public CVulkanDeviceExtensionWithFeature
	{
	public:
		CVulkanSpirvExtension()
			:CVulkanDeviceExtensionWithFeature(VK_KHR_SPIRV_1_4_EXTENSION_NAME)
		{}
	};

	class CVulkanShaderFloatControlsExtension :public CVulkanDeviceExtensionWithFeature
	{
	public:
		CVulkanShaderFloatControlsExtension()
			:CVulkanDeviceExtensionWithFeature(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME)
		{}
	};

	static std::vector<CVulkanDeviceExtensionWithFeature> extensionsWithFeatures;
	static bool isDeviceExtensionInit = false;

	std::vector<CVulkanDeviceExtensionWithFeature>& GetVulkanDeviceExtensionWithFeatureList()
	{
		if (!isDeviceExtensionInit)
		{
			extensionsWithFeatures.push_back(CVulkanAccelerationStructureExtension());
			extensionsWithFeatures.push_back(CVulkanRayTracingPipelineExtension());
			extensionsWithFeatures.push_back(CVulkanKHRBufferDeviceAddressExtension());
			//extensionsWithFeatures.push_back(CVulkanDescriptorBufferExtension());
			extensionsWithFeatures.push_back(CVulkanDefferedHostOperationExtension());
			extensionsWithFeatures.push_back(CVulkanSpirvExtension());
			extensionsWithFeatures.push_back(CVulkanShaderFloatControlsExtension());
		}
		return extensionsWithFeatures;
	}

	namespace EXT_debug_marker
	{
		bool                              IsSupported          = false;

		PFN_vkDebugMarkerSetObjectTagEXT  SetObjectTag         = nullptr;
		PFN_vkDebugMarkerSetObjectNameEXT SetObjectName        = nullptr;
		PFN_vkCmdDebugMarkerBeginEXT      CmdDebugMarkerBegin  = nullptr;
		PFN_vkCmdDebugMarkerEndEXT        CmdDebugMarkerEnd    = nullptr;
		PFN_vkCmdDebugMarkerInsertEXT     CmdDebugMarkerInsert = nullptr;
	}

	namespace EXT_rasterization_order
	{
		bool                              IsSupported          = false;
	}

	//TanGram:VSM:BEGIN
	namespace EXT_device_generated_commands
	{
		bool                              IsSupported = false;
		PFN_vkCreateIndirectCommandsLayoutNV CmdCreateIndirectCommandsLayout = nullptr;;
		PFN_vkGetGeneratedCommandsMemoryRequirementsNV CmdGetGeneratedCommandsMemoryRequirements = nullptr;
		PFN_vkCmdExecuteGeneratedCommandsNV  CmdExecuteGeneratedCommands = nullptr;
	}
	//TanGram:VSM:END

	//TanGram:VKRT:BEGIN
	namespace KHR_acceleration_structure
	{
		bool                              IsSupported = false;
		PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
		PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = nullptr;
		PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = nullptr;
		PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = nullptr;
	}
	//TanGram:VKRT:END

	void Init(CDevice* pDevice, const std::vector<const char*>& loadedExtensions)
	{
		for (auto extensionName : loadedExtensions)
		{
			if (strcmp(extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
			{
				debugMarkerObjectNameMap[pDevice->GetVkDevice()] = DeviceObjPointerNameMap();

				EXT_debug_marker::SetObjectTag         = (PFN_vkDebugMarkerSetObjectTagEXT)  vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkDebugMarkerSetObjectTagEXT");
				EXT_debug_marker::SetObjectName        = (PFN_vkDebugMarkerSetObjectNameEXT) vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkDebugMarkerSetObjectNameEXT");
				EXT_debug_marker::CmdDebugMarkerBegin  = (PFN_vkCmdDebugMarkerBeginEXT)      vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkCmdDebugMarkerBeginEXT");
				EXT_debug_marker::CmdDebugMarkerEnd    = (PFN_vkCmdDebugMarkerEndEXT)        vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkCmdDebugMarkerEndEXT");
				EXT_debug_marker::CmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)     vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkCmdDebugMarkerInsertEXT");

				EXT_debug_marker::IsSupported = true;
			}
			else if (strcmp(extensionName, VK_AMD_RASTERIZATION_ORDER_EXTENSION_NAME) == 0)
			{
				EXT_rasterization_order::IsSupported = true;
			}
			//TanGram:VSM:BEGIN
			else if (strcmp(extensionName, VK_NV_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME) == 0)
			{
				EXT_device_generated_commands::IsSupported = true;
				EXT_device_generated_commands::CmdCreateIndirectCommandsLayout = (PFN_vkCreateIndirectCommandsLayoutNV)vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkCreateIndirectCommandsLayoutNV");
				EXT_device_generated_commands::CmdGetGeneratedCommandsMemoryRequirements = (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkGetGeneratedCommandsMemoryRequirementsNV");
				EXT_device_generated_commands::CmdExecuteGeneratedCommands = (PFN_vkCmdExecuteGeneratedCommandsNV)vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkCmdExecuteGeneratedCommandsNV");
			}
			//TanGram:VSM:END
			//TanGram:VKRT:BEGIN
			else if (strcmp(extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0)
			{
				KHR_acceleration_structure::IsSupported = true;
				KHR_acceleration_structure::vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkGetAccelerationStructureBuildSizesKHR");
				KHR_acceleration_structure::vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkCreateAccelerationStructureKHR");
				KHR_acceleration_structure::vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkGetAccelerationStructureDeviceAddressKHR");
				KHR_acceleration_structure::vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkCmdBuildAccelerationStructuresKHR");
			}
			//TanGram:VKRT:END
		}

		CRY_ASSERT(KHR_acceleration_structure::IsSupported == true);
	}

	VkResult SetObjectName(VkDevice device, uintptr_t objectPtr, VkDebugMarkerObjectNameInfoEXT* pNameInfo)
	{
		if (!EXT_debug_marker::IsSupported) return VK_ERROR_EXTENSION_NOT_PRESENT;

		GetDeviceObjPointerNameMap(device)[objectPtr] = pNameInfo->pObjectName;
		return EXT_debug_marker::SetObjectName(device, pNameInfo);
	}

	DeviceObjPointerNameMap& GetDeviceObjPointerNameMap(VkDevice device)
	{
		auto deviceMarkerObjectNameMap = debugMarkerObjectNameMap.find(device);
		CRY_ASSERT(deviceMarkerObjectNameMap != debugMarkerObjectNameMap.end());
		return deviceMarkerObjectNameMap->second;
	}

	std::string GetObjectName(VkDevice device, uintptr_t objectPtr)
	{
		if (!EXT_debug_marker::IsSupported) return "";

		auto deviceMarkerObjectNameMap = GetDeviceObjPointerNameMap(device);

		auto debugName = deviceMarkerObjectNameMap.find(objectPtr);
		if(debugName != deviceMarkerObjectNameMap.end())
			return debugName->second;
		return "";
	}

	void ClearDebugName(VkDevice device, uintptr_t objectPtr)
	{
		if (!EXT_debug_marker::IsSupported) return;

		auto deviceMarkerObjectNameMap = GetDeviceObjPointerNameMap(device);

		auto debugName = deviceMarkerObjectNameMap.find(objectPtr);
		if (debugName != deviceMarkerObjectNameMap.end())
			debugName->second = "";
	}
}

}
