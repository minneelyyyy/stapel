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

#ifdef VULKAN_ENABLED
#include <vulkan/vulkan.h>
#endif

#include <cstdint>
#include <string>
#include <memory>

namespace stapel
{
    namespace backend
    {
        class IWindowBackend
        {
        public:
#ifdef VULKAN_ENABLED
            virtual VkSurfaceKHR GetVulkanSurface(VkInstance instance) = 0;
#endif

            enum Backend { Wayland, X11, Windows };
            virtual Backend GetBackendType() const = 0;
        };
    }

    class Window
    {
    public:
        Window(uint32_t width, uint32_t height, const std::string& title);
        backend::IWindowBackend& GetBackend();

    private:
        std::unique_ptr<backend::IWindowBackend> window_ = nullptr;
    };
}
