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

#include <Stapel/Stapel.h>
#include <Window/Window.h>

#ifdef USE_VULKAN
#include <Renderer/Vulkan/VulkanRenderer.h>
#endif

#include <memory>

extern "C" STAPEL_API int stapel_engine_entry(int argc, char** argv)
{
    stapel::Window::WindowSpecification spec = {
        .width = 800,
        .height = 600,
        .title = "Stapel Game",
    };

    auto window = stapel::GetWindow(spec);

    auto renderer =
        stapel::CreateBackend(window, stapel::Renderer::Backend::Vulkan, "Stapel Game", VK_MAKE_VERSION(0, 0, 1));

    while (!window->ShouldClose()) {
        window->Event();
        renderer->DrawFrame();
    }

    return 0;
}
