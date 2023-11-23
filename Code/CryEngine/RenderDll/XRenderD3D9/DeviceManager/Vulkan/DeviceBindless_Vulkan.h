#pragma once
#include "../../DeviceManager/DeviceResources.h"

enum EBindlessDescriptorBindingType
{
	e_bdstUniform_0 = 0,
	e_bdstUniform_1,
	e_bdstUniform_2,
	e_bdstUniform_3,
	e_bdstUniformNum = e_bdstUniform_3 - e_bdstUniform_0,
	e_bdbtNum
};

DEFINE_ENUM_FLAG_OPERATORS(EBindlessDescriptorBindingType);

struct SUniformBufferBindingState
{
	uint32 m_currentFreeIndex;
	std::vector<uint32>m_freeIndexArray;
	std::vector<VkBuffer> m_buffers;
};

class CDeviceBindlessDescriptorManager_Vulkan : public CDeviceBindlessDescriptorManager
{
public:
	CDeviceBindlessDescriptorManager_Vulkan();
	void Init();
	void UpdatePendingDescriptorSets();

	bool m_isUpdateDescriptor = false;
	VkDescriptorSetLayout m_bindlessDescriptorSetLayout;
	VkDescriptorSet m_bindlessDescriptorSet;
	VkDescriptorPool m_bindlessDescriptorPool;

	std::vector<VkWriteDescriptorSet> m_descriptorSetWrites;

	SUniformBufferBindingState uniformBufferBindingState[EBindlessDescriptorBindingType::e_bdstUniformNum];

private:

	void CreateBindlessDescriptorPool();
	
};