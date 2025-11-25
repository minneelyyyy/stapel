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

#include "VulkanRenderer.h"
#include "Stapel/Stapel.h"

#include <version.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#ifdef TARGET_LINUX
#   ifdef X11_ENABLED
#       include <X11/Xlib.h>
#   include <vulkan/vulkan_xlib.h>
#   endif
#   ifdef WAYLAND_ENABLED
#       include <vulkan/vulkan_wayland.h>
#   endif
#elifdef TARGET_WINDOWS
#   include <vulkan/vulkan_win32.h>
#else
#   error No valid target specified
#endif

#include <iostream>
#include <vector>
#include <algorithm>
#include <ranges>
#include <cstring>
#include <cmath>

namespace stapel::backend
{
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    #ifdef NDEBUG
            bool enableValidationLayers = false;
    #else
            bool enableValidationLayers = true;
    #endif

    std::vector<const char*> enabledValidationLayers()
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

    VkInstance CreateInstance(const Window& window, const char *name, uint32_t version)
    {
        std::vector<const char*> exts;
        exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef TARGET_LINUX
#   ifdef X11_ENABLED
        if (window.GetBackendType() == Window::X11) {
            exts.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        }
#   endif
#   ifdef WAYLAND_ENABLED
        if (window.GetBackendType() == Window::Wayland) {
            exts.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
        }
#   endif
#elifdef TARGET_WINDOWS
        if (window.GetBackendType() == Window::Windows) {
            exts.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        }
#else
#   error No valid target specified
#endif

        if (exts.size() == 0)
            STAPEL_FATAL("No surface extension");

        uint32_t count;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

        std::vector<VkExtensionProperties> extensions(count);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()) != VK_SUCCESS)
            STAPEL_FATAL("failure to get instance properties");

        VkApplicationInfo appinfo {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = name,
            .applicationVersion = version,
            .pEngineName = "Stapel",
            .engineVersion = VK_MAKE_VERSION(STAPEL_VERSION_MAJOR, STAPEL_VERSION_MINOR, STAPEL_VERSION_PATCH),
            .apiVersion = VK_API_VERSION_1_4,
        };

        auto layers = enabledValidationLayers();

        if (enableValidationLayers && layers.size() == 0)
            std::cout << "WARNING: validation layers specified (implicit from debug build), but none selected" << std::endl;

        VkInstanceCreateInfo info {
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

    VkSurfaceKHR CreateSurface(Window& window, VkInstance instance)
    {
        return window.CreateVulkanSurface(instance);
    }

    uint32_t FindQueueFamilyIndex(VkPhysicalDevice device, VkSurfaceKHR surface, VkQueueFlagBits flags, bool present)
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

    VulkanRenderer::Device VulkanRenderer::CreateDevice(VkInstance instance, VkSurfaceKHR surface)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
            STAPEL_FATAL("No Vulkan supported devices found!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // filter for valid candidates, i.e. candidates which have a surface present enabled & graphics enabled queue family
        auto candidates = devices
        | std::views::transform([&](VkPhysicalDevice phys) {
            return std::pair(phys, FindQueueFamilyIndex(phys, surface, VK_QUEUE_GRAPHICS_BIT, true));
        })
        | std::views::filter([](const auto& tup) {
            const auto& [_phys, idx] = tup;
            return idx != INDEX_INVAL;
        });

        // grab first candidate
        auto it = std::begin(candidates);
        if (it == std::end(candidates)) {
            STAPEL_FATAL("No valid Vulkan supported devices found!");
        }

        auto [phys, index] = *it;

        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT phys_feat_dyn_state_ext = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .extendedDynamicState = VK_TRUE,
        };

        VkPhysicalDeviceVulkan13Features phys_feat_vk13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &phys_feat_dyn_state_ext,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
        };

        VkPhysicalDeviceVulkan12Features phys_feat_vk12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &phys_feat_vk13,
            .descriptorIndexing = VK_TRUE,
            .bufferDeviceAddress = VK_TRUE,
        };

        VkPhysicalDeviceFeatures2 phys_feat_2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &phys_feat_vk12,
        };

        std::vector<const char*> extensions {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
            VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
        };

        float priority = 1.0f;

        VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = index,
            .queueCount = 1,
            .pQueuePriorities = &priority,
        };

        VkDeviceCreateInfo createInfo  = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &phys_feat_2,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue_create_info,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        VkDevice device;
        if (vkCreateDevice(phys, &createInfo, nullptr, &device) != VK_SUCCESS)
            STAPEL_FATAL("failed to create logical device!");

        VkQueue queue;
        vkGetDeviceQueue(device, index, 0, &queue);

        return Device {
            .phys = phys,
            .device = device,
            .queue = queue,
            .index = index,
        };
    }
 
    VkSurfaceFormatKHR SelectBestSurfaceFormat(VkSurfaceKHR surface, VkPhysicalDevice device)
    {
        uint32_t count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);

        std::vector<VkSurfaceFormatKHR> formats(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());

        for (const auto format : formats)
        {
            if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8_SRGB)
                return format;
        }

        return formats[0];
    }

    VkPresentModeKHR SelectBestPresentMode(VkPhysicalDevice device)
    {
#if 0
        uint32_t count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &count, nullptr);

        std::vector<VkPresentModeKHR> modes(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &count, modes.data());

        for (const VkPresentModeKHR mode : modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return mode;
        }
#endif

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SelectBestExtent(Window& window, VkSurfaceCapabilitiesKHR capabilities)
    {
        if (capabilities.currentExtent.width != static_cast<uint32_t>(0xffffffff)) {
            return capabilities.currentExtent;
        }

        VkExtent2D extent = {
            .width = std::clamp<uint32_t>(
                window.Width(), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height = std::clamp<uint32_t>(
                window.Height(), capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
        };

        return extent;
    }

    VulkanRenderer::Swapchain VulkanRenderer::CreateSwapchain(Window& window, VulkanRenderer::Device device, VkSurfaceKHR surface)
    {        
        auto surface_format = SelectBestSurfaceFormat(surface, device.phys);
        auto present_mode = SelectBestPresentMode(device.phys);

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.phys, surface, &capabilities);
        VkExtent2D extent = SelectBestExtent(window, capabilities);

        uint32_t min_img_count = std::max<>(3u, capabilities.minImageCount);

        if (capabilities.maxImageCount > 0 && min_img_count > capabilities.maxImageCount) {
            min_img_count = capabilities.maxImageCount;
        }

        VkFormat format = surface_format.format;

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
            .pQueueFamilyIndices = &device.index,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
        };

        VkSwapchainKHR chain;
        if (vkCreateSwapchainKHR(device.device, &info, nullptr, &chain) != VK_SUCCESS)
            STAPEL_FATAL("Failed to create swapchain");

        uint32_t count;
        vkGetSwapchainImagesKHR(device.device, chain, &count, nullptr);

        std::vector<VkImage> images(count);
        vkGetSwapchainImagesKHR(device.device, chain, &count, images.data());

        std::vector<VkImageView> image_views;
        image_views.reserve(count);

        for (VkImage image : images) {
            VkImageViewUsageCreateInfo flags = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            };

            VkImageViewCreateInfo info = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = &flags,
                .image = image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };

            VkImageView view;
            vkCreateImageView(device.device, &info, nullptr, &view);

            image_views.push_back(view);
        }

        return Swapchain {
            .chain = chain,
            .extent = extent,
            .format = format,
            .images = images,
            .image_views = image_views,
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

    void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout current, VkImageLayout next)
    {
        VkImageAspectFlags aspectMask = (next == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
            ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

        VkImageMemoryBarrier2 image_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
            .oldLayout = current,
            .newLayout = next,
            .image = image,
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = aspectMask,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
            },
        };

        VkDependencyInfo dependency_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &image_barrier,
        };

        vkCmdPipelineBarrier2(cmd, &dependency_info);
    }

    void VulkanRenderer::DrawFrame()
    {
        vkWaitForFences(device_.device, 1, &GetFrame().render_fence, true, UINT64_MAX);
        vkResetFences(device_.device, 1, &GetFrame().render_fence);

        uint32_t image_idx;
        vkAcquireNextImageKHR(device_.device, swapchain_.chain, UINT64_MAX, GetFrame().swapchain_semaphore, nullptr, &image_idx);

        VkCommandBuffer cmd = GetFrame().buffer;
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo cmd_buf_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vkBeginCommandBuffer(cmd, &cmd_buf_begin_info);

        TransitionImage(cmd, swapchain_.images[image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        VkClearColorValue clear;
        float flash = std::abs(std::sin(frame_idx_ / 120.f));
        clear = { { 0.0f, 0.0f, flash, 1.0f } };

        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        };

        vkCmdClearColorImage(cmd, swapchain_.images[image_idx], VK_IMAGE_LAYOUT_GENERAL, &clear, 1, &range);

        TransitionImage(cmd, swapchain_.images[image_idx], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        vkEndCommandBuffer(cmd);

        VkCommandBufferSubmitInfo cmd_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = cmd,
        };

        VkSemaphoreSubmitInfo wait_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = GetFrame().swapchain_semaphore,
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        };

        VkSemaphoreSubmitInfo signal_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = GetFrame().render_semaphore,
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

        if (VK_SUCCESS != vkQueueSubmit2(device_.queue, 1, &submit_info, GetFrame().render_fence))
            STAPEL_FATAL("failure to submit command queue");

        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &GetFrame().render_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain_.chain,
            .pImageIndices = &image_idx,
        };

        VkResult res = vkQueuePresentKHR(device_.queue, &present_info);

        frame_idx_++;
    }

    VulkanRenderer::VulkanRenderer(std::shared_ptr<Window> window, const char *name, uint32_t version)
        : window_(window)
    {
        instance_ = CreateInstance(*window, name, version);
        surface_ = CreateSurface(*window, instance_);

        device_ = CreateDevice(instance_, surface_);

        swapchain_ = CreateSwapchain(*window, device_, surface_);

        uint32_t size;
        vkGetSwapchainImagesKHR(device_.device, swapchain_.chain, &size, nullptr);

        frames_.reserve(size);

        for (uint32_t i = 0; i < size; i++) {
            FrameData frame;

            frame.pool = CreateCommandPool(device_.device, device_.index);
            frame.buffer = CreateCommandBuffer(device_.device, frame.pool);
            frame.render_fence = CreateFence(device_.device, true);
            frame.render_semaphore = CreateBinarySemaphore(device_.device);
            frame.swapchain_semaphore = CreateBinarySemaphore(device_.device);

            frames_.push_back(frame);
        }
    }

    Renderer::Backend VulkanRenderer::GetBackend() const {
        return Backend::Vulkan;
    }

    void VulkanRenderer::DestroySwapchain(VkDevice device)
    {
        vkDestroySwapchainKHR(device, swapchain_.chain, nullptr);

        for (auto view : swapchain_.image_views) {
            vkDestroyImageView(device, view, nullptr);
        }

        swapchain_.image_views.clear();
        swapchain_.images.clear();
    }

    VulkanRenderer::~VulkanRenderer()
    {
        vkDeviceWaitIdle(device_.device);

        for (auto& frame : frames_) {
            vkDestroyCommandPool(device_.device, frame.pool, nullptr);

            vkDestroyFence(device_.device, frame.render_fence, nullptr);
            vkDestroySemaphore(device_.device, frame.render_semaphore, nullptr);
            vkDestroySemaphore(device_.device, frame.swapchain_semaphore, nullptr);
        }

        DestroySwapchain(device_.device);

        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyDevice(device_.device, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }
}
