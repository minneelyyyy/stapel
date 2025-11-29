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

#include <Stapel/Stapel.h>

#ifdef USE_VULKAN
#include <vulkan/vulkan.h>
#endif

#include <memory>

namespace stapel
{
    class Window
    {
    public:
        struct WindowSpecification {
            int width, height;
            const char *title;
        };

        virtual ~Window() = default;

        virtual uint32_t Width() const = 0;
        virtual uint32_t Height() const = 0;

        virtual void Event() {};
        virtual bool ShouldClose() { return false; };

#ifdef USE_VULKAN
        virtual VkSurfaceKHR CreateVulkanSurface(VkInstance instance) = 0;
#endif
        enum Backend { Wayland, X11, Windows };
        virtual Backend GetBackendType() const = 0;
    };

    std::shared_ptr<Window> GetWindow(const Window::WindowSpecification& spec);
}
