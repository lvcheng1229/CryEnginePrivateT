// Copyright 2016-2021 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "VKBase.hpp"
namespace NCryVulkan
{
	namespace Extensions
	{
		class CVulkanDeviceExtension
		{
		public:
			CVulkanDeviceExtension(const char* inVkExtensionName)
				:vkExtensionName(inVkExtensionName) {}

			const char* GetExtensionName()
			{
				return vkExtensionName;
			}
		private:
			const char* vkExtensionName;
		};

		class CVulkanDeviceExtensionWithFeature : public CVulkanDeviceExtension
		{
		public:
			CVulkanDeviceExtensionWithFeature(const char* inVkExtensionName)
				:CVulkanDeviceExtension(inVkExtensionName) {}

			virtual void WritePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2KHR& PhysicalDeviceFeatures2) {};
			virtual void GetPhysicalDeviceProperties(VkPhysicalDeviceProperties2KHR& PhysicalDeviceProperties2, CDevice* pDevice){};
			virtual void EnablePhysicalDeviceFeatures(VkDeviceCreateInfo& DeviceInfo) {};
		};

		std::vector<CVulkanDeviceExtensionWithFeature>& GetVulkanDeviceExtensionWithFeatureList();

		namespace EXT_debug_marker
		{
			extern bool                              IsSupported;

			extern PFN_vkDebugMarkerSetObjectTagEXT  SetObjectTag;
			extern PFN_vkDebugMarkerSetObjectNameEXT SetObjectName;
			extern PFN_vkCmdDebugMarkerBeginEXT      CmdDebugMarkerBegin;
			extern PFN_vkCmdDebugMarkerEndEXT        CmdDebugMarkerEnd;
			extern PFN_vkCmdDebugMarkerInsertEXT     CmdDebugMarkerInsert;
		}

		namespace EXT_rasterization_order
		{
			extern bool                              IsSupported;
		}

		//TanGram:VSM:BEGIN
		namespace EXT_device_generated_commands
		{
			extern bool									IsSupported;
			extern PFN_vkCreateIndirectCommandsLayoutNV CmdCreateIndirectCommandsLayout;
			extern PFN_vkGetGeneratedCommandsMemoryRequirementsNV CmdGetGeneratedCommandsMemoryRequirements;
			extern PFN_vkCmdExecuteGeneratedCommandsNV  CmdExecuteGeneratedCommands;
		}
		//TanGram:VSM:END

		//TanGram:VKRT:BEGIN
		namespace KHR_acceleration_structure
		{
			extern  bool                              IsSupported;
			extern  PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
		}
		//TanGram:VKRT:END

		void Init(CDevice* pDevice, const std::vector<const char*>& loadedExtensions);

		typedef                  std::unordered_map<uintptr_t, std::string>   DeviceObjPointerNameMap;
		static                   std::map<VkDevice, DeviceObjPointerNameMap> debugMarkerObjectNameMap;
		VkResult                 SetObjectName              (VkDevice device, uintptr_t objectPtr, VkDebugMarkerObjectNameInfoEXT* pNameInfo);
		DeviceObjPointerNameMap& GetDeviceObjPointerNameMap (VkDevice device);
		std::string              GetObjectName              (VkDevice device, uintptr_t objectPtr);
		void                     ClearDebugName             (VkDevice device, uintptr_t objectPtr);
	}
}