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

#include "Window.h"

#ifdef USE_X11
#include "X11/X11Window.h"
#endif
#ifdef USE_WAYLAND
#include "Wayland/WlWindow.h"
#endif
#ifdef USE_WIN32
#include "Windows/WinWindow.h"
#endif

#include <memory>
#include <stdexcept>

namespace stapel
{
std::shared_ptr<Window> GetWindow(const Window::WindowSpecification& spec)
{
#ifdef TARGET_LINUX
#if defined(USE_X11) && defined(USE_WAYLAND)
    if (getenv("WAYLAND_DISPLAY")) {
        return std::make_unique<backend::wayland::Window>(spec);
    } else if (getenv("DISPLAY")) {
        return std::make_unique<backend::x11::Window>(spec);
    }
#elif defined(USE_X11)
    return std::make_unique<backend::Window>(spec);
#elif defined(USE_WAYLAND)
    return std::make_unique<backend::WaylandWindow>(spec);
#endif
#elif TARGET_WINDOWS
    return std::make_unique<backend::Win32Window>(spec);
#endif
    throw std::runtime_error("failed to create window. Are you running in a graphical environment?");
}
} // namespace stapel
