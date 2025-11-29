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
**/

#include "VulkanRenderer.h"
#include "Renderer/Vulkan/Device.h"

#include <memory>
#include <version.h>

#include <vulkan/vulkan.h>

#pragma clang diagnostic push
// VMA is particularly annoying with this flag enabled
#pragma clang diagnostic ignored "-Wnullability-completeness"

// only define VMA_IMPLEMENTATION here.
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#pragma clang diagnostic pop

#ifdef TARGET_LINUX
#ifdef USE_X11
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif
#ifdef USE_WAYLAND
#include <vulkan/vulkan_wayland.h>
#endif
#elifdef TARGET_WINDOWS
#include <vulkan/vulkan_win32.h>
#else
#error No valid target specified
#endif

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

namespace stapel::backend::vulkan
{
const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
bool enableValidationLayers = false;
#else
bool enableValidationLayers = true;
#endif

std::vector<const char*> EnabledValidationLayers()
{
    std::vector<const char*> layers;
    layers.reserve(validationLayers.size());

    if (!enableValidationLayers)
        return layers;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layer : validationLayers) {
        for (const auto prop : availableLayers) {
            if (!std::strcmp(layer, prop.layerName)) {
                layers.push_back(layer);
                break;
            }
        }
    }

    return layers;
}

VkInstance CreateInstance(const Window& window, const char* name, uint32_t version)
{
    std::vector<const char*> exts;
    exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef TARGET_LINUX
#ifdef USE_X11
    if (window.GetBackendType() == Window::X11) {
        exts.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    }
#endif
#ifdef USE_WAYLAND
    if (window.GetBackendType() == Window::Wayland) {
        exts.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
    }
#endif
#elifdef TARGET_WINDOWS
    if (window.GetBackendType() == Window::Windows) {
        exts.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    }
#else
#error No valid target specified
#endif

    if (exts.size() == 0)
        STAPEL_FATAL("No surface extension");

    uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> extensions(count);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()) != VK_SUCCESS)
        STAPEL_FATAL("failure to get instance properties");

    VkApplicationInfo appinfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = name,
        .applicationVersion = version,
        .pEngineName = "Stapel",
        .engineVersion = VK_MAKE_VERSION(STAPEL_VERSION_MAJOR, STAPEL_VERSION_MINOR, STAPEL_VERSION_PATCH),
        .apiVersion = VK_API_VERSION_1_4,
    };

    auto layers = EnabledValidationLayers();

    if (enableValidationLayers && layers.size() == 0)
        std::cout << "WARNING: validation layers specified (implicit from debug build), but none selected" << std::endl;

    VkInstanceCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appinfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(exts.size()),
        .ppEnabledExtensionNames = exts.data(),
    };

    VkInstance instance;
    if (vkCreateInstance(&info, nullptr, &instance) != VK_SUCCESS)
        STAPEL_FATAL("Could not instantiate Vulkan");

    return instance;
}

VkSurfaceFormatKHR SelectBestSurfaceFormat(VkSurfaceKHR surface, VkPhysicalDevice device)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());

    for (const auto format : formats) {
        if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8_SRGB)
            return format;
    }

    return formats[0];
}

std::optional<VkPresentModeKHR> SelectBestPresentMode(VkPhysicalDevice device, VkSurfaceKHR surface,
                                                      std::span<VkPresentModeKHR> preferred_modes)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);

    // no modes available, hopefully this does not happen.
    if (count == 0)
        return {};

    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());

    for (auto preferred_mode : preferred_modes) {
        for (auto mode : modes) {
            if (mode == preferred_mode)
                return mode;
        }
    }

    // no preferred mode found, use the first one available
    return modes.front();
}

VkExtent2D SelectBestExtent(Window& window, VkSurfaceCapabilitiesKHR capabilities)
{
    if (capabilities.currentExtent.width != static_cast<uint32_t>(0xffffffff)) {
        return capabilities.currentExtent;
    }

    VkExtent2D extent = {
        .width =
            std::clamp<uint32_t>(window.Width(), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        .height = std::clamp<uint32_t>(window.Height(), capabilities.minImageExtent.height,
                                       capabilities.maxImageExtent.height),
    };

    return extent;
}

Renderer::Swapchain Renderer::CreateSwapchain(Window& window, vulkan::Device& device, VkSurfaceKHR surface)
{
    VkSurfaceFormatKHR surface_format = SelectBestSurfaceFormat(surface, device.GetPhys());

    VkPresentModeKHR mode_pref[] = {
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR,
    };

    VkPresentModeKHR present_mode = SelectBestPresentMode(device.GetPhys(), surface, mode_pref).value();

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhys(), surface, &capabilities);
    VkExtent2D extent = SelectBestExtent(window, capabilities);

    uint32_t min_img_count = std::max<>(3u, capabilities.minImageCount);

    if (capabilities.maxImageCount > 0 && min_img_count > capabilities.maxImageCount) {
        min_img_count = capabilities.maxImageCount;
    }

    VkFormat format = surface_format.format;

    uint32_t index = device.GetFamilyIndex();

    VkSwapchainCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .flags = 0,
        .surface = surface,
        .minImageCount = min_img_count,
        .imageFormat = format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &index,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkSwapchainKHR chain;
    if (vkCreateSwapchainKHR(device.GetDevice(), &info, nullptr, &chain) != VK_SUCCESS)
        STAPEL_FATAL("Failed to create swapchain");

    uint32_t count;
    vkGetSwapchainImagesKHR(device.GetDevice(), chain, &count, nullptr);

    std::vector<VkImage> vkImages(count);
    vkGetSwapchainImagesKHR(device.GetDevice(), chain, &count, vkImages.data());

    std::vector<Image> images;
    images.reserve(count);

    for (VkImage image : vkImages) {
        Image img =
            Image::Wrap(device.GetDevice(), extent.width, extent.height, 1, format, image, VK_IMAGE_LAYOUT_UNDEFINED);
        images.push_back(std::move(img));
    }

    return Swapchain{
        .chain = chain,
        .images = std::move(images),
    };
}

VkCommandPool CreateCommandPool(VkDevice device, uint32_t queue_family_index)
{
    VkCommandPoolCreateInfo cmd_pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_index,
    };

    VkCommandPool pool;
    if (vkCreateCommandPool(device, &cmd_pool_info, nullptr, &pool) != VK_SUCCESS)
        STAPEL_FATAL("Failed to create command pool");

    return pool;
}

VkCommandBuffer CreateCommandBuffer(VkDevice device, VkCommandPool pool)
{
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer buffer;
    if (vkAllocateCommandBuffers(device, &alloc_info, &buffer) != VK_SUCCESS)
        STAPEL_FATAL("Failed to allocate command buffer");

    return buffer;
}

VkSemaphore CreateBinarySemaphore(VkDevice device)
{
    VkSemaphoreCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkSemaphore semaphore;
    if (vkCreateSemaphore(device, &info, nullptr, &semaphore) != VK_SUCCESS)
        STAPEL_FATAL("Failed to create semaphore");

    return semaphore;
}

VkFence CreateFence(VkDevice device, bool signaled)
{
    VkFenceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    if (signaled)
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence;
    if (vkCreateFence(device, &info, nullptr, &fence) != VK_SUCCESS)
        STAPEL_FATAL("Failed to create fence");

    return fence;
}

void Renderer::DrawFrame()
{
    FrameData& frame = GetFrame();

    vkWaitForFences(device_->GetDevice(), 1, &frame.render_fence, true, UINT64_MAX);
    vkResetFences(device_->GetDevice(), 1, &frame.render_fence);

    frame.del.DeleteAll();

    uint32_t image_idx;
    vkAcquireNextImageKHR(device_->GetDevice(), swapchain_.chain, UINT64_MAX, frame.swapchain_semaphore, nullptr,
                          &image_idx);

    Image& img = swapchain_.images[image_idx];

    VkCommandBuffer cmd = frame.buffer;
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo cmd_buf_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd, &cmd_buf_begin_info);

    img.Transition(cmd, VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clear;
    float flash = std::abs(std::sin(frame_idx_ / 120.f));
    clear = {{0.0f, 0.0f, flash, 1.0f}};

    VkImageSubresourceRange range = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };

    vkCmdClearColorImage(cmd, swapchain_.images[image_idx].GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clear, 1, &range);

    img.Transition(cmd, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    vkEndCommandBuffer(cmd);

    VkCommandBufferSubmitInfo cmd_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
    };

    VkSemaphoreSubmitInfo wait_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.swapchain_semaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
    };

    VkSemaphoreSubmitInfo signal_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.render_semaphore,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
    };

    VkSubmitInfo2 submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &wait_info,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmd_info,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signal_info,
    };

    if (VK_SUCCESS != vkQueueSubmit2(device_->GetQueue(), 1, &submit_info, frame.render_fence))
        STAPEL_FATAL("failure to submit command queue");

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame.render_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchain_.chain,
        .pImageIndices = &image_idx,
    };

    VkResult res = vkQueuePresentKHR(device_->GetQueue(), &present_info);

    frame_idx_++;
}

Renderer::Renderer(std::shared_ptr<Window> window, const char* name, uint32_t version) : window_(window)
{
    instance_ = CreateInstance(*window, name, version);
    surface_ = window->CreateVulkanSurface(instance_);

    auto devs = vulkan::DeviceBuilder(instance_, surface_)
                    .ext(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                    .ext(VK_KHR_SPIRV_1_4_EXTENSION_NAME)
                    .extendedDynamicState(true)
                    .synchronization2(true)
                    .dynamicRendering(true)
                    .descriptorIndexing(true)
                    .bufferDeviceAddress(true)
                    .Devices();

    if (devs.size() == 0)
        STAPEL_FATAL("No valid device found!");

    device_ = std::make_unique<vulkan::Device>(instance_, surface_, devs[0]);

    swapchain_ = CreateSwapchain(*window, *device_, surface_);

    uint32_t size;
    vkGetSwapchainImagesKHR(device_->GetDevice(), swapchain_.chain, &size, nullptr);

    frames_.reserve(size);

    for (uint32_t i = 0; i < size; i++) {
        FrameData frame;

        frame.pool = CreateCommandPool(device_->GetDevice(), device_->GetFamilyIndex());
        frame.buffer = CreateCommandBuffer(device_->GetDevice(), frame.pool);
        frame.render_fence = CreateFence(device_->GetDevice(), true);
        frame.render_semaphore = CreateBinarySemaphore(device_->GetDevice());
        frame.swapchain_semaphore = CreateBinarySemaphore(device_->GetDevice());

        frames_.push_back(frame);
    }

    VmaAllocatorCreateInfo allocatorInfo = {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = device_->GetPhys(),
        .device = device_->GetDevice(),
        .instance = instance_,
    };

    vmaCreateAllocator(&allocatorInfo, &alloc_);

    del_.Push([&]() { vmaDestroyAllocator(alloc_); });
}

stapel::Renderer::Backend Renderer::GetBackend() const
{
    return stapel::Renderer::Backend::Vulkan;
}

void Renderer::DestroySwapchain(VkDevice device)
{
    vkDestroySwapchainKHR(device, swapchain_.chain, nullptr);
    swapchain_.images.clear();
}

Renderer::~Renderer()
{
    vkDeviceWaitIdle(device_->GetDevice());

    for (auto& frame : frames_) {
        vkDestroyCommandPool(device_->GetDevice(), frame.pool, nullptr);

        vkDestroyFence(device_->GetDevice(), frame.render_fence, nullptr);
        vkDestroySemaphore(device_->GetDevice(), frame.render_semaphore, nullptr);
        vkDestroySemaphore(device_->GetDevice(), frame.swapchain_semaphore, nullptr);

        frame.del.DeleteAll();
    }

    del_.DeleteAll();

    DestroySwapchain(device_->GetDevice());

    device_.reset();

    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);
}
} // namespace stapel::backend::vulkan
