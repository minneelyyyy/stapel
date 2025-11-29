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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef USE_VULKAN
#include <vulkan/vulkan.h>
#endif

namespace stapel::backend::win32
{
class Window : public stapel::Window
{
public:
    Window(const Specification& spec);
    ~Window() override;

    uint32_t Width() const override;
    uint32_t Height() const override;

    void Event() override;

#ifdef USE_VULKAN
    VkSurfaceKHR CreateVulkanSurface(VkInstance instance) override;
#endif

    Window::Backend GetBackendType() const override
    {
        return Window::Windows;
    }

private:
    HINSTANCE instance_;
    HWND hwnd_;
};
} // namespace stapel::backend::win32
