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

#include <X11/X.h>
#include <X11/Xlib.h>

#include "X11Window.h"

#include <stdexcept>
#include <vulkan/vulkan_xlib.h>

namespace stapel::backend
{
    X11Window::X11Window(uint32_t width, uint32_t height, const std::string& title)
    {
        display_ = XOpenDisplay(NULL);
    
        if (!display_) {
            throw std::runtime_error("failed to open X display.");
        }

        window_ = XCreateSimpleWindow(
            display_,
            XDefaultRootWindow(display_),
            0, 0,
            width, height,
            0,
            0x0, 0x0);

        XStoreName(display_, window_, title.c_str());

        XSelectInput(display_, window_, KeyPressMask | KeyReleaseMask);
        XMapWindow(display_, window_);
    }

#ifdef VULKAN_ENABLED
    VkSurfaceKHR X11Window::GetVulkanSurface(VkInstance instance)
    {
        VkXlibSurfaceCreateInfoKHR info {};
        info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        info.dpy = display_;
        info.window = window_;

        VkSurfaceKHR surface;
        if (vkCreateXlibSurfaceKHR(instance, &info, nullptr, &surface) != VK_SUCCESS)
            return nullptr;

        return surface;
    }
#endif

    X11Window::~X11Window()
    {
        XCloseDisplay(display_);
    }
}
