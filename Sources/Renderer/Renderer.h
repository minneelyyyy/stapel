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

#pragma once

#include <Stapel/Stapel.h>
#include <Window/Window.h>

#include <memory>

namespace stapel
{
class Renderer
{
public:
    enum Backend { Vulkan };

public:
    static std::unique_ptr<Renderer> getRenderer(std::shared_ptr<Window> window, Backend api, const char* name,
                                                 uint32_t major, uint32_t minor, uint32_t patch);
    virtual ~Renderer() = default;

    virtual Backend backend() const = 0;

    virtual void drawFrame() = 0;
};
} // namespace stapel
