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

#pragma once

#include "Renderer/Vulkan/DeletionQueue.h"
#include <Renderer/Renderer.h>
#include <Stapel/Stapel.h>

#include "Device.h"
#include "Image.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

#include "vk_mem_alloc.h"

namespace stapel::backend::vulkan
{

class DescriptorAllocator
{
public:
    struct PoolSizeRatio {
        VkDescriptorType type;
        float ratio;
    };

public:
    DescriptorAllocator(VkDevice device, uint32_t max_sets, std::span<PoolSizeRatio> pool_ratios);
    ~DescriptorAllocator();

    void clearDescriptors();
    void destroyPool();

    VkDescriptorSet allocate(VkDescriptorSetLayout layout);

private:
    VkDevice device_;
    VkDescriptorPool pool_;
};

class Renderer : public stapel::Renderer
{
public:
    Renderer(std::shared_ptr<Window> window, const char* name, uint32_t version);
    ~Renderer();

    stapel::Renderer::Backend backend() const;

    void drawBackground(VkCommandBuffer cmd, Image& img);
    void drawFrame();

private:
    struct FrameData {
        VkCommandPool pool;
        VkCommandBuffer buffer;
        VkSemaphore swapchain_semaphore, render_semaphore;
        VkFence render_fence;
        DeletionQueue del;
    };

    struct Swapchain {
        VkSwapchainKHR chain;
        std::vector<Image> images;
        VkExtent3D extent;
        VkFormat format;
    };

    const static unsigned int FRAME_OVERLAP = 2;

private:
    static Swapchain createSwapchain(Window& window, vulkan::Device& device, VkSurfaceKHR surface);

    void destroySwapchain(VkDevice device);

    FrameData& getFrame()
    {
        return frames_[frame_idx_ % frames_.size()];
    };

private:
    std::shared_ptr<Window> window_;
    VkInstance instance_;
    VkSurfaceKHR surface_;
    std::unique_ptr<vulkan::Device> device_;

    Swapchain swapchain_;
    std::optional<Image> draw_img_;

    std::vector<FrameData> frames_;
    unsigned int frame_idx_ = 0;

    DeletionQueue del_;
    VmaAllocator alloc_;

    std::unique_ptr<DescriptorAllocator> descriptor_alloc_;

    VkDescriptorSet draw_image_desc_;
    VkDescriptorSetLayout draw_image_desc_layout_;

    VkPipeline gradient_pipeline_;
    VkPipelineLayout gradient_pipeline_layout_;
};
} // namespace stapel::backend::vulkan
