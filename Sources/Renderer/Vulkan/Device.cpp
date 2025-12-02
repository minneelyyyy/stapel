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
#include "util.h"

#include <Stapel/Stapel.h>

#include <vulkan/vulkan.h>

#include <cstring>

namespace stapel::backend::vulkan
{
DeviceConfig::DeviceConfig()
{
    feat.pNext = &vk13;
    vk13.pNext = &vk12;
    vk12.pNext = &dynam;
    dynam.pNext = &maint1;
}

static VkDevice create_device(const PhysicalDeviceInfo& info)
{
    float priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = info.family_idx,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &info.cfg->feat,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_create_info,
        .enabledExtensionCount = static_cast<uint32_t>(info.cfg->exts.size()),
        .ppEnabledExtensionNames = info.cfg->exts.data(),
    };

    VkDevice device;
    if (vkCreateDevice(info.phys, &create_info, nullptr, &device) != VK_SUCCESS)
        STAPEL_FATAL("failed to create logical device!");

    return device;
}

static VkQueue get_device_queue(VkDevice device, uint32_t idx)
{
    VkQueue queue;
    vkGetDeviceQueue(device, idx, 0, &queue);

    return queue;
}

Device::Frame::Frame(Device& device)
    : device_(device),
      image_available_(util::create_binary_semaphore(device_.device_)),
      in_flight_(util::create_fence(device.device_, true)),
      pool_(device.device_, device.idx_),
      cmd_(device.device_, pool_.pool_)
{}

Device::Frame::~Frame()
{
    vkDestroyFence(device_.device_, in_flight_, nullptr);
    vkDestroySemaphore(device_.device_, image_available_, nullptr);
}

CommandBuffer& Device::Frame::getCmdBuffer()
{
    return cmd_;
}

Image& Device::Frame::getSwapchainImage()
{
    return device_.swapchain_->images_[image_idx_];
}

std::unique_ptr<Swapchain> create_swapchain(Window& win, VkDevice device, VkPhysicalDevice phys,
                                            uint32_t idx, VkSurfaceKHR surface)
{
    return std::make_unique<Swapchain>(device, phys, idx, surface,
                                       SwapchainSpec{
                                           .format =
                                               {
                                                   .format = VK_FORMAT_B8G8R8A8_SRGB,
                                                   .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                                               },
                                           .modes =
                                               {
                                                   VK_PRESENT_MODE_FIFO_KHR,
                                                   VK_PRESENT_MODE_MAILBOX_KHR,
                                                   VK_PRESENT_MODE_IMMEDIATE_KHR,
                                               },
                                           .width = win.width(),
                                           .height = win.height(),
                                       });
}

Device::Device(Window& window, VkInstance instance, VkSurfaceKHR surface,
               const PhysicalDeviceInfo& info)
    : window_(window),
      phys_(info.phys),
      surface_(surface),
      idx_(info.family_idx),
      device_(create_device(info)),
      queue_(get_device_queue(device_, idx_))
{
    swapchain_ = create_swapchain(window, device_, phys_, idx_, surface_);

    frames_.reserve(N_FRAMES_IN_FLIGHT);

    for (int i = 0; i < N_FRAMES_IN_FLIGHT; i++) {
        frames_.emplace_back(std::make_unique<Device::Frame>(*this));
    }

    images_in_flight_.resize(swapchain_->images_.size());

    render_finished_semaphores_.resize(swapchain_->images_.size());

    for (size_t i = 0; i < swapchain_->images_.size(); i++) {
        render_finished_semaphores_[i] = util::create_binary_semaphore(device_);
    }
}

Device::~Device()
{
    for (auto& frame : frames_) {
        frame.reset();
    }

    for (VkSemaphore sem : render_finished_semaphores_) {
        vkDestroySemaphore(device_, sem, nullptr);
    }

    swapchain_.reset();
    vkDestroyDevice(device_, nullptr);
}

uint32_t Device::currentFrame()
{
    return frame_idx_ % N_FRAMES_IN_FLIGHT;
}

void Device::waitIdle()
{
    vkDeviceWaitIdle(device_);
}

void Device::rebuildSwapchain()
{
    waitIdle();
    swapchain_.reset();
    swapchain_ = create_swapchain(window_, device_, phys_, idx_, surface_);
}

Device::Frame& Device::acquireNextFrame()
{
    Frame& frame = *frames_[currentFrame()];

    vkWaitForFences(device_, 1, &frame.in_flight_, VK_TRUE, UINT64_MAX);

    switch (vkAcquireNextImageKHR(device_, swapchain_->swapchain_, UINT64_MAX,
                                  frame.image_available_, VK_NULL_HANDLE, &frame.image_idx_)) {
    case VK_SUCCESS:
        break;
    case VK_SUBOPTIMAL_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
        rebuildSwapchain();
        break;
    default:
        STAPEL_FATAL("failure to acquire swapchain image");
    };

    if (images_in_flight_[frame.image_idx_] != VK_NULL_HANDLE) {
        vkWaitForFences(device_, 1, &images_in_flight_[frame.image_idx_], VK_TRUE, UINT64_MAX);
    }

    images_in_flight_[frame.image_idx_] = frame.in_flight_;

    vkResetFences(device_, 1, &frame.in_flight_);

    return frame;
}

void Device::submitCmdBuffer(const CommandBuffer& cmd, Frame& frame)
{
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame.image_available_,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd.buf_,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &render_finished_semaphores_[frame.image_idx_],
    };

    if (VK_SUCCESS != vkQueueSubmit(queue_, 1, &submit_info, frame.in_flight_))
        STAPEL_FATAL("failure to submit command queue");
}

void Device::present(Frame& frame)
{
    VkFence dummy = VK_NULL_HANDLE;

    VkSwapchainPresentFenceInfoEXT ext = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT,
        .swapchainCount = 1,
        .pFences = &dummy,
    };

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = &ext,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render_finished_semaphores_[frame.image_idx_],
        .swapchainCount = 1,
        .pSwapchains = &swapchain_->swapchain_,
        .pImageIndices = &frame.image_idx_,
    };

    switch (vkQueuePresentKHR(queue_, &present_info)) {
    case VK_SUCCESS:
        break;
    case VK_SUBOPTIMAL_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
        rebuildSwapchain();
        break;
    default:
        STAPEL_FATAL("queue present error");
    }

    frame_idx_++;
}

DeviceBuilder::DeviceBuilder(VkInstance instance, VkSurfaceKHR surface)
    : instance_(instance), surface_(surface), cfg_(std::make_shared<DeviceConfig>())
{}

DeviceBuilder::~DeviceBuilder()
{}

bool check_device_extensions(std::vector<VkExtensionProperties> exts, const char* ext)
{
    for (auto x : exts) {
        if (!::strcmp(x.extensionName, ext))
            return true;
    }

    return false;
}

bool check_device_extensions(VkPhysicalDevice dev, std::vector<const char*> exts)
{
    uint32_t size;
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &size, nullptr);

    std::vector<VkExtensionProperties> dev_exts(size);
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &size, dev_exts.data());

    for (auto ext : exts) {
        if (!check_device_extensions(dev_exts, ext))
            return false;
    }

    return true;
}

bool DeviceBuilder::checkDeviceFeatures(VkPhysicalDevice dev)
{
    VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT maint1 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT,
    };

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynam = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .pNext = &maint1,
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

#define X(st, name)                                                                                \
    if (cfg_->st.name && !st.name)                                                                 \
        return false;

#include "vk_features.xdefs"
#undef X

    return true;
}

uint32_t find_queue_family_index(VkPhysicalDevice device, VkSurfaceKHR surface,
                                 VkQueueFlagBits flags, bool present)
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & flags) {
            VkBool32 present;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present);

            if (present)
                return i;
        }
    }

    return INDEX_INVAL;
}

std::vector<PhysicalDeviceInfo> DeviceBuilder::devices()
{
    uint32_t size;

    vkEnumeratePhysicalDevices(instance_, &size, nullptr);

    std::vector<VkPhysicalDevice> devs(size);
    vkEnumeratePhysicalDevices(instance_, &size, devs.data());

    std::vector<PhysicalDeviceInfo> infos;
    infos.reserve(size);

    for (auto dev : devs) {
        if (!check_device_extensions(dev, cfg_->exts) || !checkDeviceFeatures(dev))
            continue;

        uint32_t idx = find_queue_family_index(dev, surface_, VK_QUEUE_GRAPHICS_BIT, true);

        PhysicalDeviceInfo info = {
            .phys = dev,
            .family_idx = idx,
            .cfg = cfg_,
        };

        infos.push_back(info);
    }

    return infos;
}
} // namespace stapel::backend::vulkan
