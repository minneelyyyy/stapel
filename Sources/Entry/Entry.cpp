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

#include <Stapel/Stapel.h>
#include <Window/Window.h>
#include <Renderer/Vulkan/VulkanRenderer.h>

#include <iostream>
#include <memory>

extern stapel::IApplication* GetApplication();

extern "C" STAPEL_API
int stapel_engine_entry(int argc, char **argv)
{
    stapel::IApplication* app = GetApplication();
    if (!app)
        STAPEL_FATAL("Failed to load application");

    app->PreEngineInitHook(argc, argv);

    stapel::WindowSpecification winspec {};
    app->WindowCreateSpecHook(winspec);

    auto window = stapel::GetWindow(winspec);

    stapel::RendererSpecification spec {
        .backend = stapel::backend::Vulkan_1_4,
        .app = app->GetApplicationInfo(),
    };

    auto renderer = stapel::CreateBackend(window, spec);
    renderer->Init(spec.app);

    return 0;
}
