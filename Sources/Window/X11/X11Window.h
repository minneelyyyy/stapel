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

#include <Window/Window.h>

#include <X11/Xlib.h>

namespace stapel::backend
{
    class X11Window : public Window
    {
    public:
        X11Window(const WindowSpecification& spec);
        ~X11Window() override;

        uint32_t Width() const override { return width_; }
        uint32_t Height() const override { return height_; }

#ifdef USE_VULKAN
        VkSurfaceKHR CreateVulkanSurface(VkInstance instance) override;
#endif

        Window::Backend GetBackendType() const override { return Window::X11; }

    private:
        ::Display* display_;
        ::Window window_;
        uint32_t width_, height_;
    };
}
