/* Copyright 2025 minneelyyyy <abigail@minneelyyyy.dev>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <Renderer/Renderer.h>

#include <vulkan/vulkan.h>

#include <optional>

namespace stapel::backend
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    class VulkanDevice
    {
    public:
        VulkanDevice(VkInstance instance, VkSurfaceKHR surface);
        ~VulkanDevice();

        const QueueFamilyIndices& GetQueueFamilyIndices() const { return indices_; };
        VkQueue GetGraphicsQueue() const { return graphicsQueue_; };
        VkDevice GetVulkanDevice() const { return device_; };
        VkPhysicalDevice GetVulkanPhysicalDevice() const { return physDevice_; };

    private:
        VkPhysicalDevice physDevice_;
        VkDevice device_;
        QueueFamilyIndices indices_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;
    };
}
