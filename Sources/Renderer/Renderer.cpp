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

#include "Renderer.h"

#include <Stapel/Stapel.h>

#ifdef VULKAN_ENABLED
#include "Vulkan/VulkanRenderer.h"
#endif

#include <memory>
#include <stdexcept>

namespace stapel
{
    Renderer::Renderer(std::shared_ptr<Window> window, backend::BackendAPI api, ApplicationInfo app)
        : window_(window)
    {
        switch (api)
        {
            case backend::BackendAPI::Vulkan_1_3:
#ifdef VULKAN_ENABLED
                backend_ = std::make_unique<backend::VulkanRenderer>(window_, app);
#else
                throw std::runtime_error("Vulkan renderer API selected but support not compiled into engine.");
#endif
                break;
            default:
                throw std::runtime_error("Invalid/unsupported API selected.");
        }
    }
}
