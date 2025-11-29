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

#include "Image.h"

namespace stapel::backend::vulkan
{
static VkImageView create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageViewType view_type,
                                     VkImageAspectFlags flags)
{
    VkImageView view;

    VkImageViewCreateInfo img_view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = view_type,
        .format = format,
        .subresourceRange =
            {
                .aspectMask = flags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    vkCreateImageView(device, &img_view_info, nullptr, &view);

    return view;
}

Image::Image(VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImage image,
             VkImageLayout layout, VkImageView view)
    : device_(device), extent_({width, height, depth}), format_(format), image_(image), current_layout_(layout),
      allocator_(nullptr), allocation_(nullptr)
{
    view_ = view ? view : create_image_view(device_, image_, format_, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
}

Image::Image(VmaAllocator alloc, VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format)
    : allocator_(alloc), device_(device), extent_(width, height, depth), format_(format),
      current_layout_(VK_IMAGE_LAYOUT_UNDEFINED)
{
    VkImageUsageFlags draw_img_use = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                     VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo img_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format_,
        .extent = extent_,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = draw_img_use,
    };

    VmaAllocationCreateInfo vma_alloc_info = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    vmaCreateImage(allocator_, &img_info, &vma_alloc_info, &image_, &allocation_, nullptr);

    view_ = create_image_view(device_, image_, format_, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
}

Image Image::wrap(VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImage image,
                  VkImageLayout layout, VkImageView view)
{
    Image img = Image(device, width, height, depth, format, image, layout, view);
    img.owns_image_ = false;

    if (view)
        img.owns_view_ = false;

    return img;
}

void Image::transition(VkCommandBuffer cmd, VkImageLayout layout, std::optional<VkImageLayout> old_layout)
{
    VkImageAspectFlags aspect_mask =
        (layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
        .oldLayout = old_layout.has_value() ? old_layout.value() : current_layout_,
        .newLayout = layout,
        .image = image_,
        .subresourceRange =
            VkImageSubresourceRange{
                .aspectMask = aspect_mask,
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
    current_layout_ = layout;
}

void Image::copy(VkCommandBuffer cmd, Image& dest)
{
    VkImageBlit2 blit_region = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
        .srcSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .srcOffsets = {{0, 0, 0},
                       {
                           static_cast<int32_t>(extent_.width),
                           static_cast<int32_t>(extent_.height),
                           static_cast<int32_t>(extent_.depth),
                       }},
        .dstSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .dstOffsets = {{0, 0, 0},
                       {
                           static_cast<int32_t>(dest.extent_.width),
                           static_cast<int32_t>(dest.extent_.height),
                           static_cast<int32_t>(dest.extent_.depth),
                       }},
    };

    VkBlitImageInfo2 blit_info = {
        .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
        .srcImage = image_,
        .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .dstImage = dest.image_,
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .regionCount = 1,
        .pRegions = &blit_region,
        .filter = VK_FILTER_LINEAR,
    };

    vkCmdBlitImage2(cmd, &blit_info);
}

void Image::clear()
{
    if (owns_view_) {
        vkDestroyImageView(device_, view_, nullptr);
    }

    if (owns_image_) {
        vkDestroyImage(device_, image_, nullptr);

        // we only want to call vmaFreeMemory if this image
        // is allocated by VMA
        if (allocation_) {
            vmaFreeMemory(allocator_, allocation_);
        }
    }
}

Image::~Image()
{
    clear();
}
} // namespace stapel::backend::vulkan
