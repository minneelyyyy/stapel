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

#include <Stapel/Stapel.h>
#include <Renderer/Renderer.h>
#include "Renderer/Vulkan/DeletionQueue.h"

#include "Device.h"
#include "Image.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

#include "vk_mem_alloc.h"

namespace stapel::backend::vulkan
{
    class Renderer : public stapel::Renderer
    {
    public:
        Renderer(std::shared_ptr<Window> window, const char *name, uint32_t version);
        ~Renderer();

        stapel::Renderer::Backend GetBackend() const;

        void DrawFrame();

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
        };

        const static unsigned int FRAME_OVERLAP = 2;

    private:
        static Swapchain CreateSwapchain(Window& window, vulkan::Device& device, VkSurfaceKHR surface);

        void DestroySwapchain(VkDevice device);

        FrameData& GetFrame() { return frames_[frame_idx_ % frames_.size()]; };

    private:
        std::shared_ptr<Window> window_;
        VkInstance instance_;
        VkSurfaceKHR surface_;
        std::unique_ptr<vulkan::Device> device_;

        Swapchain swapchain_;

        std::vector<FrameData> frames_;
        unsigned int frame_idx_ = 0;

        DeletionQueue del_;
        VmaAllocator alloc_;
    };
}
