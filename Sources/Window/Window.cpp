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

#include "Window.h"

#ifdef TARGET_LINUX
#   ifdef X11_ENABLED
#       include "X11/X11Window.h"
#   endif
#   ifdef WAYLAND_ENABLED
#       include "Wayland/WlWindow.h"
#   endif
#elif TARGET_WINDOWS
#   include "Windows/WinWindow.h"
#endif

#include <memory>
#include <stdexcept>

namespace stapel
{
    Window::Window(const WindowSpecification& spec)
    {
#ifdef TARGET_LINUX
#   if defined(X11_ENABLED) && defined(WAYLAND_ENABLED)
        if (getenv("WAYLAND_DISPLAY")) {
            window_ = std::make_unique<backend::WaylandWindow>(spec);
        } else if (getenv("DISPLAY")) {
            window_ = std::make_unique<backend::X11Window>(spec);
        }
#   elif defined(X11_ENABLED)
        window_ = std::make_unique<backend::X11Window>(spec);
#   elif defined(WAYLAND_ENABLED)
        window_ = std::make_unique<backend::WaylandWindow>(spec);
#   endif
#elif TARGET_WINDOWS
        window_ = std::make_unique<backend::Win32Window>(spec);
#endif
        if (!window_)
            throw std::runtime_error("failed to create window. Are you running in a graphical environment?");
    }

    backend::IWindowBackend& Window::GetBackend()
    {
        return *window_;
    }
}
