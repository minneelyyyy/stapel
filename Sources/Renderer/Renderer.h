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

#pragma once

#include <Window/Window.h>

#include <Stapel/Stapel.h>

#include <memory>

namespace stapel
{
    namespace backend
    {
        enum BackendAPI {
            Vulkan_1_3,
        };

        class IRendererBackend
        {
        };
    }

    class Renderer
    {
    public:
        Renderer(std::shared_ptr<Window> window, backend::BackendAPI api, ApplicationInfo app);

    private:
        std::shared_ptr<Window> window_;
        std::unique_ptr<backend::IRendererBackend> backend_;
    };
}
