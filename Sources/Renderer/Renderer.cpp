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

#include <Renderer/Renderer.h>
#include <Renderer/Vulkan/VulkanRenderer.h>

#include <Window/Window.h>

#include <stdexcept>

namespace stapel
{
    std::unique_ptr<Renderer> CreateBackend(std::shared_ptr<Window> window, const RendererSpecification& spec)
    {
        switch (spec.backend) {
        case backend::Vulkan_1_4:
#ifdef VULKAN_ENABLED
            return std::make_unique<backend::VulkanRenderer>(window);
#else
            throw std::runtime_error("Vulkan selected as backend but support is not built in to engine");
#endif
        default:
            throw std::runtime_error("No/Invalid renderer backend selected.");
        }
    }
}
