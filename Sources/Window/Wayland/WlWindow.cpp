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

#include "WlWindow.h"
#include "Stapel/Stapel.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-protocol.h"

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>

#include <stdexcept>
#include <cstring>

static void wm_base_ping(void* data, xdg_wm_base* wm_base, uint32_t serial)
{
    xdg_wm_base_pong(wm_base, serial);
}

static void xdg_surface_configure(void* data, xdg_surface* surface, uint32_t serial)
{
    auto* state = static_cast<stapel::backend::WaylandState*>(data);

    xdg_surface_ack_configure(surface, serial);

    wl_surface_commit(state->surface);
}

static void xdg_toplevel_configure(void* data, xdg_toplevel* toplevel, int32_t width, int32_t height, wl_array *array)
{
    auto* state = static_cast<stapel::backend::WaylandState*>(data);

    if (width > 0 && height > 0) {
        state->width = width;
        state->height = height;

        state->should_rebuild_swapchain = true;
    }
}

static void xdg_toplevel_close(void* data, xdg_toplevel* toplevel)
{
    auto* state = static_cast<stapel::backend::WaylandState*>(data);
    state->should_close = true;
}

static const xdg_wm_base_listener wm_base_listener = {
    .ping = wm_base_ping,
};

static const xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static const xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
};

static void registry_handle_global(void* data, struct wl_registry* registry,
	uint32_t name, const char *interface, uint32_t version)
{
    auto* state = static_cast<stapel::backend::WaylandState*>(data);

    if (!::strcmp(interface, wl_compositor_interface.name))
    {
        state->compositor = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, 4));
    }
    else if (!::strcmp(interface, xdg_wm_base_interface.name))
    {
        state->xdg_base = static_cast<xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));

        xdg_wm_base_add_listener(state->xdg_base, &wm_base_listener, state);
    }
    else if (!::strcmp(interface, zxdg_decoration_manager_v1_interface.name))
    {
        state->zxdg_decoration_manager = static_cast<zxdg_decoration_manager_v1*>(
            wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1));
    }
}

static void registry_handle_global_remove(void* data, struct wl_registry* registry, uint32_t name)
{
}

static const struct wl_registry_listener listener {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

namespace stapel::backend
{
    WaylandWindow::WaylandWindow(const WindowSpecification& spec)
    {
        state_.width = spec.width;
        state_.height = spec.height;

        display_ = wl_display_connect(nullptr);

        if (!display_)
            throw std::runtime_error("could not connect to Wayland display.");

        wl_registry* registry = wl_display_get_registry(display_);

        if (!registry)
            throw std::runtime_error("failed to get registry from display.");

        wl_registry_add_listener(registry, &listener, static_cast<void*>(&state_));
        wl_display_roundtrip(display_);

        if (!state_.compositor)
            STAPEL_FATAL("Failure to get Wayland compositor");

        state_.surface = wl_compositor_create_surface(state_.compositor);
        if (!state_.surface)
            STAPEL_FATAL("failed to create wl_surface");

        if (state_.xdg_base) {
            state_.xdg_surface = xdg_wm_base_get_xdg_surface(state_.xdg_base, state_.surface);
            xdg_surface_add_listener(state_.xdg_surface, &xdg_surface_listener, &state_);

            state_.xdg_toplevel = xdg_surface_get_toplevel(state_.xdg_surface);
            xdg_toplevel_add_listener(state_.xdg_toplevel, &xdg_toplevel_listener, &state_);

            xdg_toplevel_set_title(state_.xdg_toplevel, spec.title);
            xdg_toplevel_set_min_size(state_.xdg_toplevel, state_.width, state_.height);
            xdg_toplevel_set_app_id(state_.xdg_toplevel, "stapel");

            if (state_.zxdg_decoration_manager) {
                state_.toplevel_decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
                    state_.zxdg_decoration_manager, state_.xdg_toplevel);

                zxdg_toplevel_decoration_v1_set_mode(state_.toplevel_decoration,
                    ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
            }
            wl_display_roundtrip(display_);
        }

        wl_surface_commit(state_.surface);
        wl_display_roundtrip(display_);
    }

    WaylandWindow::~WaylandWindow()
    {
        if (state_.toplevel_decoration) zxdg_toplevel_decoration_v1_destroy(state_.toplevel_decoration);
        if (state_.zxdg_decoration_manager) zxdg_decoration_manager_v1_destroy(state_.zxdg_decoration_manager);
        if (state_.xdg_toplevel) xdg_toplevel_destroy(state_.xdg_toplevel);
        if (state_.xdg_surface) xdg_surface_destroy(state_.xdg_surface);
        if (state_.surface) wl_surface_destroy(state_.surface);
        if (state_.xdg_base) xdg_wm_base_destroy(state_.xdg_base);
        if (state_.compositor) wl_compositor_destroy(state_.compositor);
        if (display_) wl_display_disconnect(display_);
    }

    void WaylandWindow::Event()
    {
        while (wl_display_prepare_read(display_) != 0)
            wl_display_dispatch_pending(display_);
    
        wl_display_flush(display_);
    
        wl_display_read_events(display_);
        wl_display_dispatch_pending(display_);
    }

#ifdef VULKAN_ENABLED
    VkSurfaceKHR WaylandWindow::CreateVulkanSurface(VkInstance instance)
    {
        VkWaylandSurfaceCreateInfoKHR info = {
            .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
            .display = display_,
            .surface = state_.surface,
        };

        VkSurfaceKHR vk_surface;
        if (vkCreateWaylandSurfaceKHR(instance, &info, nullptr, &vk_surface) != VK_SUCCESS)
            return nullptr;

        return vk_surface;
    }
#endif
}
