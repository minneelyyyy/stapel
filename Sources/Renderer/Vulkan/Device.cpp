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

#include "Device.h"

#include <Stapel/Stapel.h>

#include <cstring>
#include <vulkan/vulkan_core.h>

namespace stapel::backend::vulkan
{
    DeviceConfig::DeviceConfig()
    {
        feat.pNext = &vk13;
        vk13.pNext = &vk12;
        vk12.pNext = &dynam;
    }

    Device::Device(VkInstance instance, VkSurfaceKHR surface, const PhysicalDeviceInfo& info)
        : phys_(info.phys), idx_(info.family_idx)
    {
        float priority = 1.0f;

        VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = info.family_idx,
            .queueCount = 1,
            .pQueuePriorities = &priority,
        };

        VkDeviceCreateInfo createInfo  = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &info.cfg->feat,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue_create_info,
            .enabledExtensionCount = static_cast<uint32_t>(info.cfg->exts.size()),
            .ppEnabledExtensionNames = info.cfg->exts.data(),
        };

        if (vkCreateDevice(info.phys, &createInfo, nullptr, &device_) != VK_SUCCESS)
            STAPEL_FATAL("failed to create logical device!");

        vkGetDeviceQueue(device_, idx_, 0, &queue_);
    }

    Device::~Device()
    {
        vkDestroyDevice(device_, nullptr);
    }

    DeviceBuilder::DeviceBuilder(VkInstance instance, VkSurfaceKHR surface)
        : instance_(instance), surface_(surface), cfg_(std::make_shared<DeviceConfig>())
    {
    }

    DeviceBuilder::~DeviceBuilder()
    {
    }

    bool checkForExtension(std::vector<VkExtensionProperties> exts, const char *ext)
    {
        for (auto x : exts) {
            if (!::strcmp(x.extensionName, ext))
                return true;
        }

        return false;
    }

    bool checkDeviceExtensions(VkPhysicalDevice dev, std::vector<const char*> exts)
    {
        uint32_t size;
        vkEnumerateDeviceExtensionProperties(dev, nullptr, &size, nullptr);

        std::vector<VkExtensionProperties> dev_exts(size);
        vkEnumerateDeviceExtensionProperties(dev, nullptr, &size, dev_exts.data());

        for (auto ext : exts) {
            if (!checkForExtension(dev_exts, ext))
                return false;
        }

        return true;
    }

    bool DeviceBuilder::CheckDeviceFeatures(VkPhysicalDevice dev)
    {
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynam = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        };

        VkPhysicalDeviceVulkan12Features vk12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &dynam,
        };

        VkPhysicalDeviceVulkan13Features vk13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &vk12,
        };

        VkPhysicalDeviceFeatures2 dev_feat = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &vk13,
        };

        vkGetPhysicalDeviceFeatures2(dev, &dev_feat);

#define X(st, name) \
        if (cfg_->st.name && !st.name) return false;

        _VULKAN_FEATURE_XDEFS
#undef X

        return true;
    }

    uint32_t findQueueFamilyIndex(VkPhysicalDevice device, VkSurfaceKHR surface, VkQueueFlagBits flags, bool present)
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & flags) {
                VkBool32 present;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present);
    
                if (present)
                    return i;
            }
        }

        return INDEX_INVAL;
    }

    std::vector<PhysicalDeviceInfo> DeviceBuilder::Devices()
    {
        uint32_t size;

        vkEnumeratePhysicalDevices(instance_, &size, nullptr);

        std::vector<VkPhysicalDevice> devs(size);
        vkEnumeratePhysicalDevices(instance_, &size, devs.data());

        std::vector<PhysicalDeviceInfo> infos;
        infos.reserve(size);

        for (auto dev : devs) {
            if (!checkDeviceExtensions(dev, cfg_->exts) || !CheckDeviceFeatures(dev))
                continue;

            uint32_t idx = findQueueFamilyIndex(dev, surface_, VK_QUEUE_GRAPHICS_BIT, true);

            PhysicalDeviceInfo info = {
                .phys = dev,
                .family_idx = idx,
                .cfg = cfg_,
            };

            infos.push_back(info);
        }

        return infos;
    }
}
