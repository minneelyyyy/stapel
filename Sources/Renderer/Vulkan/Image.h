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

#include <iostream>
#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"

namespace stapel::backend::vulkan
{
class Image
{
  public:
    Image(VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImage image,
          VkImageLayout layout, VkImageView view = nullptr);
    Image(VmaAllocator alloc, VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format);

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&& other) noexcept
        : owns_image_(other.owns_image_), owns_view_(other.owns_view_), allocator_(other.allocator_),
          device_(other.device_), image_(other.image_), view_(other.view_), extent_(other.extent_),
          current_layout_(other.current_layout_), format_(other.format_), allocation_(other.allocation_)
    {
        other.owns_image_ = false;
        other.owns_view_ = false;
        other.image_ = nullptr;
        other.view_ = nullptr;
        other.allocation_ = nullptr;
    }

    Image& operator=(Image&& other) noexcept
    {
        if (this != &other) {
            Clear();

            owns_image_ = other.owns_image_;
            owns_view_ = other.owns_view_;
            allocator_ = other.allocator_;
            device_ = other.device_;
            image_ = other.image_;
            view_ = other.view_;
            extent_ = other.extent_;
            current_layout_ = other.current_layout_;
            format_ = other.format_;
            allocation_ = other.allocation_;

            other.owns_image_ = false;
            other.owns_view_ = false;
            other.image_ = nullptr;
            other.view_ = nullptr;
            other.allocation_ = nullptr;
        }

        return *this;
    }

    void Clear();
    ~Image();

    VkImage GetImage() const
    {
        return image_;
    }
    VkImageView GetView() const
    {
        return view_;
    }

    static Image Wrap(VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImage image,
                      VkImageLayout layout, VkImageView view = nullptr);

    void Transition(VkCommandBuffer cmd, VkImageLayout layout);
    void Copy(VkCommandBuffer cmd, Image& dest);

  private:
    bool owns_image_ = true;
    bool owns_view_ = true;
    VmaAllocator allocator_;
    VkDevice device_;
    VkImage image_;
    VkImageView view_;
    VkExtent3D extent_;
    VkImageLayout current_layout_;
    VkFormat format_;
    VmaAllocation allocation_;
};
} // namespace stapel::backend::vulkan