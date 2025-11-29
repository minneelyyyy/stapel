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
std::shared_ptr<Window> Window::GetWindow(const Window::Specification& spec)
{
#ifdef USE_WAYLAND
    if (getenv("WAYLAND_DISPLAY")) {
        return std::make_shared<backend::wayland::Window>(spec);
    }
#endif

#ifdef USE_X11
    if (getenv("DISPLAY")) {
        return std::make_shared<backend::x11::Window>(spec);
    }
#endif

#ifdef USE_WIN32
    return std::make_shared<backend::win32::Window>(spec);
#endif

#if !defined(USE_WAYLAND) && !defined(USE_X11) && !defined(USE_WIN32)
    STAPEL_FATAL("This engine was compiled without support for any windowing system");
#else
    STAPEL_FATAL("No window could be created");
#endif
}
} // namespace stapel
