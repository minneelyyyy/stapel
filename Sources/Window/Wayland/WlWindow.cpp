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

#include <wayland-client.h>
#include <vulkan/vulkan_wayland.h>

#include <stdexcept>
#include <cstring>

static void registry_handle_global(void *data, struct wl_registry *registry,
	uint32_t name, const char *interface, uint32_t version)
{
    stapel::backend::WaylandState *state = static_cast<stapel::backend::WaylandState*>(data);

    if (!::strcmp(interface, wl_compositor_interface.name))
    {
        state->compositor = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, 4));
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
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
        display_ = wl_display_connect(nullptr);

        if (!display_)
            throw std::runtime_error("could not connect to Wayland display.");

        wl_registry* registry = wl_display_get_registry(display_);

        if (!registry)
            throw std::runtime_error("failed to get registry from display.");

        wl_registry_add_listener(registry, &listener, static_cast<void*>(&state_));
        wl_display_roundtrip(display_);
    }

    WaylandWindow::~WaylandWindow()
    {
        wl_display_disconnect(display_);
    }

#ifdef VULKAN_ENABLED
    VkSurfaceKHR WaylandWindow::CreateVulkanSurface(VkInstance instance)
    {
        wl_surface* surface = wl_compositor_create_surface(state_.compositor);
        if (!surface)
            return nullptr;

        VkWaylandSurfaceCreateInfoKHR info = {
            .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
            .display = display_,
            .surface = surface,
        };

        VkSurfaceKHR vk_surface;
        if (vkCreateWaylandSurfaceKHR(instance, &info, nullptr, &vk_surface) != VK_SUCCESS)
            return nullptr;

        return vk_surface;
    }
#endif
}
