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

#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Image.h"
#include "Pipeline.h"
#include "Swapchain.h"

#include <Window/Window.h>

#include <memory>
#include <vector>

namespace stapel::backend::vulkan
{
struct DeviceConfig {
    std::vector<const char*> exts;

    VkPhysicalDeviceFeatures2 feat = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    };

    VkPhysicalDeviceVulkan12Features vk12 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
    };

    VkPhysicalDeviceVulkan13Features vk13 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    };

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
    Device(Window& window, VkInstance instance, VkSurfaceKHR surface,
           const PhysicalDeviceInfo& info);
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(Device&&) = delete;

public:
    VkDevice device()
    {
        return device_;
    }

    VkPhysicalDevice physical()
    {
        return phys_;
    }

public:
    class Frame
    {
    private:
        Device& device_;
        VkSemaphore image_available_;
        VkFence in_flight_;
        CommandPool pool_;
        CommandBuffer cmd_;
        uint32_t image_idx_;

    public:
        Frame(Device& device);
        ~Frame();

        Frame(const Frame& other) = delete;
        Frame operator=(const Frame& other) = delete;
        Frame(Frame&& other) = delete;
        Frame operator=(Frame&& other) = delete;

    public:
        CommandBuffer& getCmdBuffer();
        Image& getSwapchainImage();

    public:
        friend class Device;
    };

public:
    void waitIdle();
    Frame& acquireNextFrame();
    void submitCmdBuffer(const CommandBuffer& cmd, Frame& frame);
    void present(Frame& frame);

public:
    friend class Swapchain;
    friend class CommandPool;
    friend class CommandBuffer;
    friend class Pipeline;
    friend class ComputePipeline;

private:
    uint32_t currentFrame();

private:
    const static unsigned int N_FRAMES_IN_FLIGHT = 2;

private:
    Window& window_;
    VkPhysicalDevice phys_;
    VkSurfaceKHR surface_;
    uint32_t idx_;

    VkDevice device_;
    VkQueue queue_;

    std::unique_ptr<Swapchain> swapchain_;

    std::vector<std::unique_ptr<Frame>> frames_;
    std::vector<VkFence> images_in_flight_;
    std::vector<VkSemaphore> render_finished_semaphores_;

    uint32_t frame_idx_ = 0;
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

#define X(st, name)                                                                                \
    inline DeviceBuilder& name(bool x)                                                             \
    {                                                                                              \
        this->cfg_->st.name = x;                                                                   \
        return *this;                                                                              \
    }

#include "vk_features.xdefs"
#undef X

    bool checkDeviceFeatures(VkPhysicalDevice dev);
    std::vector<PhysicalDeviceInfo> devices();

private:
    VkInstance instance_;
    VkSurfaceKHR surface_;
    std::shared_ptr<DeviceConfig> cfg_;
};
} // namespace stapel::backend::vulkan
