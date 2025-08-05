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
#include "VulkanDevice.h"

#include <version.h>

#include <vulkan/vulkan.h>

#ifdef LINUX_TARGET
#   ifdef X11_ENABLED
#       include <X11/Xlib.h>
#   include <vulkan/vulkan_xlib.h>
#   endif
#   ifdef WAYLAND_ENABLED
#       include <vulkan/vulkan_wayland.h>
#   endif
#else
#   include <vulkan/vulkan_win32.h>
#endif

#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <optional>
#include <limits>

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
        std::vector<const char*> layers(validationLayers.size());

        if (!enableValidationLayers)
            return layers;

        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layer : validationLayers)
        {
            for (const auto prop : availableLayers)
            {
                if (!strcmp(layer, prop.layerName))
                {
                    layers.push_back(layer);
                    break;
                }
            }
        }

        return layers;
    }

    void VulkanRenderer::CreateInstance(const ApplicationInfo& app)
    {
        std::vector<const char*> exts;
        exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef LINUX_TARGET
#   ifdef X11_ENABLED
        if (window_->GetBackendType() == Window::X11) {
            exts.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        }
#   endif
#   ifdef WAYLAND_ENABLED
        if (window_->GetBackendType() == Window::Wayland) {
            exts.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
        }
#   endif
#else
        if (window_->GetBackendType() == Window::Windows) {
            exts.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        }
#endif

        uint32_t count;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

        std::vector<VkExtensionProperties> extensions(count);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()) != VK_SUCCESS)
            throw std::runtime_error("failure to get instance properties");

        std::cout << "Vulkan Extensions:" << std::endl;
        for (const auto& ext : extensions)
        {
            std::cout << '\t' << ext.extensionName << std::endl;
        }

        VkApplicationInfo appinfo {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = app.name.c_str(),
            .applicationVersion = VK_MAKE_VERSION(app.v_major, app.v_minor, app.v_patch),
            .pEngineName = "Stapel",
            .engineVersion = VK_MAKE_VERSION(STAPEL_VERSION_MAJOR, STAPEL_VERSION_MINOR, STAPEL_VERSION_PATCH),
            .apiVersion = VK_API_VERSION_1_3,
        };

        auto validationLayers = enabledValidationLayers();

        VkInstanceCreateInfo info {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appinfo,
            .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
            .ppEnabledLayerNames = validationLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(exts.size()),
            .ppEnabledExtensionNames = exts.data(),
        };

        if (vkCreateInstance(&info, nullptr, &instance_) != VK_SUCCESS)
            throw std::runtime_error("could not create Vulkan instance.");
    }

    void VulkanRenderer::CreateSurface()
    {
        surface_ = window_->CreateVulkanSurface(instance_);
    }

    void VulkanRenderer::SelectDevice()
    {
        device_ = std::make_unique<VulkanDevice>(instance_, surface_);
    }
 
    VkSurfaceFormatKHR VulkanRenderer::SelectBestSurfaceFormat()
    {
        auto device = device_->GetVulkanPhysicalDevice();

        uint32_t count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &count, nullptr);

        std::vector<VkSurfaceFormatKHR> formats(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &count, formats.data());

        for (const auto format : formats)
        {
            if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8_SRGB)
                return format;
        }

        return formats[0];
    }

    VkPresentModeKHR VulkanRenderer::SelectBestPresentMode()
    {
        auto device = device_->GetVulkanPhysicalDevice();

        uint32_t count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &count, nullptr);

        std::vector<VkPresentModeKHR> modes(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &count, modes.data());

        for (const VkPresentModeKHR mode : modes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return mode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanRenderer::SelectBestExtent()
    {
        auto device = device_->GetVulkanPhysicalDevice();

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &capabilities);

        if (capabilities.currentExtent.width != static_cast<uint32_t>(0xffffffff)) {
            return capabilities.currentExtent;
        }

        VkExtent2D extent = {
            .width = std::clamp<uint32_t>(window_->Width(), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height = std::clamp<uint32_t>(window_->Height(), capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
        };

        return extent;
    }

    void VulkanRenderer::CreateSwapBuffer()
    {
    }

    void VulkanRenderer::Init(const ApplicationInfo& app)
    {
        CreateInstance(app);
        CreateSurface();
        SelectDevice();
        CreateSwapBuffer();
    }

    VulkanRenderer::VulkanRenderer(std::shared_ptr<Window> window)
        : window_(window)
    {
    }

    VulkanRenderer::~VulkanRenderer()
    {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }
}
