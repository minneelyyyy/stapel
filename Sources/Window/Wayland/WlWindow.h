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

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include "xdg-decoration-unstable-v1-protocol.h"
#include "xdg-shell-client-protocol.h"

namespace stapel::backend::wayland
{
struct WaylandState {
    ::wl_compositor* compositor = nullptr;
    ::wl_surface* surface = nullptr;
    ::xdg_wm_base* xdg_base = nullptr;
    ::xdg_surface* xdg_surface = nullptr;
    ::xdg_toplevel* xdg_toplevel = nullptr;
    ::zxdg_decoration_manager_v1* zxdg_decoration_manager = nullptr;
    ::zxdg_toplevel_decoration_v1* toplevel_decoration = nullptr;

    bool should_close = false;
    bool should_rebuild_swapchain = false;
    uint32_t width, height;
};

class Window : public stapel::Window
{
public:
    Window(const Specification& spec);
    ~Window() override;

    uint32_t Width() const override
    {
        return state_.width;
    }

    uint32_t Height() const override
    {
        return state_.height;
    }

    void Event() override;
    bool ShouldClose() override
    {
        return state_.should_close;
    };

#ifdef USE_VULKAN
    VkSurfaceKHR CreateVulkanSurface(VkInstance instance) override;
#endif
    Window::Backend GetBackendType() const override
    {
        return Window::Wayland;
    }

private:
    ::wl_display* display_;
    WaylandState state_;
};
} // namespace stapel::backend::wayland
