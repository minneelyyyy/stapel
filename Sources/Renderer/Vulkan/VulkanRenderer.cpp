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

#include "VulkanRenderer.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#ifdef X11_ENABLED
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif

#ifdef WAYLAND_ENABLED
#include <vulkan/vulkan_wayland.h>
#endif

#include <version.h>

namespace stapel::backend
{
    VulkanRenderer::VulkanRenderer(std::shared_ptr<Window> window, ApplicationInfo app)
        : window_(window)
    {
        auto& backend = window_->GetBackend();
        std::vector<const char*> exts;

#ifdef X11_ENABLED
        if (backend.GetBackendType() == IWindowBackend::X11) {
            exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            exts.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        }
#endif

#ifdef WAYLAND_ENABLED
        if (backend.GetBackendType() == IWindowBackend::Wayland) {
            exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            exts.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
        }
#endif

        VkApplicationInfo appinfo {};
        appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appinfo.pApplicationName = app.name_.c_str();
        appinfo.applicationVersion = VK_MAKE_VERSION(app.v_major, app.v_minor, app.v_patch);
        appinfo.pEngineName = "Stapel";
        appinfo.engineVersion = VK_MAKE_VERSION(STAPEL_VERSION_MAJOR, STAPEL_VERSION_MINOR, STAPEL_VERSION_PATCH);
        appinfo.apiVersion = VK_API_VERSION_1_4;

        VkInstanceCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.enabledExtensionCount = exts.size();
        info.ppEnabledExtensionNames = exts.data();
        info.pApplicationInfo = &appinfo;

        if (vkCreateInstance(&info, nullptr, &instance_) != VK_SUCCESS)
            throw std::runtime_error("could not create Vulkan instance.");

        surface_ = backend.GetVulkanSurface(instance_);
    }

    VulkanRenderer::~VulkanRenderer()
    {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }
}
