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

#include "Swapchain.h"

#include <algorithm>
#include <optional>
#include <span>

namespace stapel::backend::vulkan
{
static std::optional<VkSurfaceFormatKHR> select_best_surface_format(VkPhysicalDevice device,
                                                                    VkSurfaceKHR surface,
                                                                    VkSurfaceFormatKHR preferred)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);

    if (count == 0)
        return {};

    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());

    for (const auto format : formats) {
        if (format.format == preferred.format && format.colorSpace == preferred.colorSpace)
            return format;
    }

    // prefererd format not found, use first available
    return formats.front();
}

static std::optional<VkPresentModeKHR>
select_best_present_mode(VkPhysicalDevice device, VkSurfaceKHR surface,
                         std::span<const VkPresentModeKHR> preferred_modes)
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

static VkExtent2D select_best_extent(VkSurfaceCapabilitiesKHR capabilities,
                                     const SwapchainSpec& spec)
{
    if (capabilities.currentExtent.width != static_cast<uint32_t>(0xffffffff)) {
        return capabilities.currentExtent;
    }

    VkExtent2D extent = {
        .width = std::clamp<uint32_t>(spec.width, capabilities.minImageExtent.width,
                                      capabilities.maxImageExtent.width),
        .height = std::clamp<uint32_t>(spec.height, capabilities.minImageExtent.height,
                                       capabilities.maxImageExtent.height),
    };

    return extent;
}

Swapchain::Swapchain(VkDevice dev, VkPhysicalDevice phys, uint32_t idx, VkSurfaceKHR surface,
                     const SwapchainSpec& spec)
    : device_(dev)
{
    VkSurfaceFormatKHR format = *select_best_surface_format(phys, surface, spec.format);
    VkPresentModeKHR mode = *select_best_present_mode(phys, surface, spec.modes);

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys, surface, &capabilities);

    VkExtent2D extent = select_best_extent(capabilities, spec);

    uint32_t min_img_count = std::max<>(3u, capabilities.minImageCount);

    if (capabilities.maxImageCount > 0 && min_img_count > capabilities.maxImageCount) {
        min_img_count = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .flags = 0,
        .surface = surface,
        .minImageCount = min_img_count,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &idx,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = mode,
        .clipped = VK_TRUE,
    };

    vkCreateSwapchainKHR(device_, &info, nullptr, &swapchain_);

    uint32_t count;
    vkGetSwapchainImagesKHR(device_, swapchain_, &count, nullptr);

    std::vector<VkImage> vk_images(count);
    vkGetSwapchainImagesKHR(device_, swapchain_, &count, vk_images.data());

    images_.reserve(count);

    for (VkImage image : vk_images) {
        Image img = Image::wrap(device_, extent.width, extent.height, 1, format.format, image,
                                VK_IMAGE_LAYOUT_UNDEFINED);

        images_.push_back(std::move(img));
    }
}

Swapchain::~Swapchain()
{
    vkDestroySwapchainKHR(device_, swapchain_, nullptr);
}
} // namespace stapel::backend::vulkan