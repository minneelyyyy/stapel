/* Copyright 2025 minneelyyyy <abigail@minneelyyyy.dev>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#define _VULKAN_FEATURE_XDEFS                                                                                          \
    X(vk13, robustImageAccess)                                                                                         \
    X(vk13, inlineUniformBlock)                                                                                        \
    X(vk13, descriptorBindingInlineUniformBlockUpdateAfterBind)                                                        \
    X(vk13, pipelineCreationCacheControl)                                                                              \
    X(vk13, privateData)                                                                                               \
    X(vk13, shaderDemoteToHelperInvocation)                                                                            \
    X(vk13, shaderTerminateInvocation)                                                                                 \
    X(vk13, subgroupSizeControl)                                                                                       \
    X(vk13, computeFullSubgroups)                                                                                      \
    X(vk13, synchronization2)                                                                                          \
    X(vk13, textureCompressionASTC_HDR)                                                                                \
    X(vk13, shaderZeroInitializeWorkgroupMemory)                                                                       \
    X(vk13, dynamicRendering)                                                                                          \
    X(vk13, shaderIntegerDotProduct)                                                                                   \
    X(vk13, maintenance4)                                                                                              \
    X(vk12, samplerMirrorClampToEdge)                                                                                  \
    X(vk12, drawIndirectCount)                                                                                         \
    X(vk12, storageBuffer8BitAccess)                                                                                   \
    X(vk12, uniformAndStorageBuffer8BitAccess)                                                                         \
    X(vk12, storagePushConstant8)                                                                                      \
    X(vk12, shaderBufferInt64Atomics)                                                                                  \
    X(vk12, shaderSharedInt64Atomics)                                                                                  \
    X(vk12, shaderFloat16)                                                                                             \
    X(vk12, shaderInt8)                                                                                                \
    X(vk12, descriptorIndexing)                                                                                        \
    X(vk12, shaderInputAttachmentArrayDynamicIndexing)                                                                 \
    X(vk12, shaderUniformTexelBufferArrayDynamicIndexing)                                                              \
    X(vk12, shaderStorageTexelBufferArrayDynamicIndexing)                                                              \
    X(vk12, shaderUniformBufferArrayNonUniformIndexing)                                                                \
    X(vk12, shaderSampledImageArrayNonUniformIndexing)                                                                 \
    X(vk12, shaderStorageBufferArrayNonUniformIndexing)                                                                \
    X(vk12, shaderStorageImageArrayNonUniformIndexing)                                                                 \
    X(vk12, shaderInputAttachmentArrayNonUniformIndexing)                                                              \
    X(vk12, shaderUniformTexelBufferArrayNonUniformIndexing)                                                           \
    X(vk12, shaderStorageTexelBufferArrayNonUniformIndexing)                                                           \
    X(vk12, descriptorBindingUniformBufferUpdateAfterBind)                                                             \
    X(vk12, descriptorBindingSampledImageUpdateAfterBind)                                                              \
    X(vk12, descriptorBindingStorageImageUpdateAfterBind)                                                              \
    X(vk12, descriptorBindingStorageBufferUpdateAfterBind)                                                             \
    X(vk12, descriptorBindingUniformTexelBufferUpdateAfterBind)                                                        \
    X(vk12, descriptorBindingStorageTexelBufferUpdateAfterBind)                                                        \
    X(vk12, descriptorBindingUpdateUnusedWhilePending)                                                                 \
    X(vk12, descriptorBindingPartiallyBound)                                                                           \
    X(vk12, descriptorBindingVariableDescriptorCount)                                                                  \
    X(vk12, runtimeDescriptorArray)                                                                                    \
    X(vk12, samplerFilterMinmax)                                                                                       \
    X(vk12, scalarBlockLayout)                                                                                         \
    X(vk12, imagelessFramebuffer)                                                                                      \
    X(vk12, uniformBufferStandardLayout)                                                                               \
    X(vk12, shaderSubgroupExtendedTypes)                                                                               \
    X(vk12, separateDepthStencilLayouts)                                                                               \
    X(vk12, hostQueryReset)                                                                                            \
    X(vk12, timelineSemaphore)                                                                                         \
    X(vk12, bufferDeviceAddress)                                                                                       \
    X(vk12, bufferDeviceAddressCaptureReplay)                                                                          \
    X(vk12, bufferDeviceAddressMultiDevice)                                                                            \
    X(vk12, vulkanMemoryModel)                                                                                         \
    X(vk12, vulkanMemoryModelDeviceScope)                                                                              \
    X(vk12, vulkanMemoryModelAvailabilityVisibilityChains)                                                             \
    X(vk12, shaderOutputViewportIndex)                                                                                 \
    X(vk12, shaderOutputLayer)                                                                                         \
    X(vk12, subgroupBroadcastDynamicId)                                                                                \
    X(dynam, extendedDynamicState)

namespace stapel::backend::vulkan
{
struct DeviceConfig {
    std::vector<const char*> exts;

    VkPhysicalDeviceFeatures2 feat = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

    VkPhysicalDeviceVulkan12Features vk12 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};

    VkPhysicalDeviceVulkan13Features vk13 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynam = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
    };

    DeviceConfig();
};

struct PhysicalDeviceInfo {
    VkPhysicalDevice phys;
    uint32_t family_idx;
    std::shared_ptr<DeviceConfig> cfg;
};

class Device
{
public:
    Device(VkInstance instance, VkSurfaceKHR surface, const PhysicalDeviceInfo& info);
    ~Device();

    VkPhysicalDevice GetPhys() const
    {
        return phys_;
    }
    VkDevice device() const
    {
        return device_;
    }
    uint32_t GetFamilyIndex() const
    {
        return idx_;
    }
    VkQueue GetQueue() const
    {
        return queue_;
    }

private:
    VkPhysicalDevice phys_;
    VkDevice device_;
    uint32_t idx_;
    VkQueue queue_;
};

class DeviceBuilder
{
public:
    DeviceBuilder(VkInstance instance, VkSurfaceKHR surface);
    ~DeviceBuilder();

    inline DeviceBuilder& ext(const char* ext)
    {
        this->cfg_->exts.push_back(ext);
        return *this;
    }

#define X(st, name)                                                                                                    \
    inline DeviceBuilder& name(bool x)                                                                                 \
    {                                                                                                                  \
        this->cfg_->st.name = x;                                                                                       \
        return *this;                                                                                                  \
    }

    _VULKAN_FEATURE_XDEFS
#undef X

    bool checkDeviceFeatures(VkPhysicalDevice dev);
    std::vector<PhysicalDeviceInfo> devices();

private:
    VkInstance instance_;
    VkSurfaceKHR surface_;
    std::shared_ptr<DeviceConfig> cfg_;
};
} // namespace stapel::backend::vulkan
